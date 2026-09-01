#include "network/filter_service.h"

#include "utils/logging.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QVariantMap>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace pb::network {
namespace {

constexpr int kMaxListBytes = 12 * 1024 * 1024;

QString categoryName(pb::net::RuleCategory category)
{
    switch (category) {
    case pb::net::RuleCategory::Ads:
        return QStringLiteral("ads");
    case pb::net::RuleCategory::Trackers:
        return QStringLiteral("trackers");
    case pb::net::RuleCategory::Other:
        break;
    }
    return QStringLiteral("other");
}

} // namespace

FilterService::FilterService(QObject *parent)
    : QObject(parent)
{
    loadBuiltInLists();
    reload();
}

FilterService::~FilterService() = default;

void FilterService::loadBuiltInLists()
{
    FilterList trackers;
    trackers.id = QStringLiteral("built-in-trackers");
    trackers.title = tr("Built-in tracker list");
    trackers.path = QStringLiteral(":/filters/trackers.txt");
    trackers.category = pb::net::RuleCategory::Trackers;
    trackers.builtIn = true;

    FilterList ads;
    ads.id = QStringLiteral("built-in-ads");
    ads.title = tr("Built-in advertising list");
    ads.path = QStringLiteral(":/filters/ads.txt");
    ads.category = pb::net::RuleCategory::Ads;
    ads.builtIn = true;

    m_lists = { trackers, ads };
}

QString FilterService::readList(const FilterList &list, bool *ok) const
{
    if (ok)
        *ok = false;

    if (m_downloaded.contains(list.id)) {
        if (ok)
            *ok = true;
        return m_downloaded.value(list.id);
    }

    QFile file(list.path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        pb::log::write(pb::log::Level::Warning, "a filter list could not be read");
        return {};
    }
    if (file.size() > kMaxListBytes) {
        pb::log::write(pb::log::Level::Warning, "a filter list was rejected for being too large");
        return {};
    }
    if (ok)
        *ok = true;
    return QString::fromUtf8(file.readAll());
}

void FilterService::rebuildEngine()
{
    pb::net::FilterEngine engine;
    QList<FilterList> lists = m_lists;

    for (FilterList &list : lists) {
        list.rules = 0;
        list.skipped = 0;
        if (!list.enabled)
            continue;
        if (list.category == pb::net::RuleCategory::Trackers && !m_trackersEnabled)
            continue;
        if (list.category == pb::net::RuleCategory::Ads && !m_adsEnabled)
            continue;

        bool ok = false;
        const QString text = readList(list, &ok);
        if (!ok)
            continue;

        const pb::net::LoadReport report
            = engine.addRules(text.toStdString(), list.category);
        list.rules = static_cast<int>(report.rules);
        list.skipped = static_cast<int>(report.skipped());
    }

    {
        QWriteLocker locker(&m_lock);
        m_engine = std::move(engine);
        m_lists = lists;
    }
    Q_EMIT listsChanged();
}

void FilterService::reload()
{
    rebuildEngine();
}

void FilterService::setEnabled(bool trackers, bool ads)
{
    if (m_trackersEnabled == trackers && m_adsEnabled == ads)
        return;
    m_trackersEnabled = trackers;
    m_adsEnabled = ads;
    rebuildEngine();
}

pb::net::MatchResult FilterService::match(const pb::net::Request &request) const
{
    QReadLocker locker(&m_lock);
    return m_engine.match(request);
}

int FilterService::ruleCount() const
{
    QReadLocker locker(&m_lock);
    return static_cast<int>(m_engine.ruleCount() + m_engine.exceptionCount());
}

QVariantList FilterService::listsAsVariants() const
{
    QReadLocker locker(&m_lock);
    QVariantList result;
    for (const FilterList &list : m_lists) {
        QVariantMap entry;
        entry[QStringLiteral("id")] = list.id;
        entry[QStringLiteral("title")] = list.title;
        entry[QStringLiteral("source")]
            = list.sourceUrl.isEmpty() ? tr("Shipped with the browser") : list.sourceUrl.toString();
        entry[QStringLiteral("category")] = categoryName(list.category);
        entry[QStringLiteral("builtIn")] = list.builtIn;
        entry[QStringLiteral("enabled")] = list.enabled;
        entry[QStringLiteral("rules")] = list.rules;
        entry[QStringLiteral("skipped")] = list.skipped;
        result.append(entry);
    }
    return result;
}

QStringList FilterService::filterListHosts() const
{
    QReadLocker locker(&m_lock);
    QStringList hosts;
    for (const FilterList &list : m_lists) {
        if (!list.sourceUrl.isEmpty() && !hosts.contains(list.sourceUrl.host()))
            hosts.append(list.sourceUrl.host());
    }
    return hosts;
}

bool FilterService::addLocalList(const QUrl &fileUrl)
{
    const QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString();
    if (path.isEmpty() || !QFileInfo::exists(path))
        return false;

    FilterList list;
    list.id = QStringLiteral("local-") + QString::number(qHash(path), 16);
    list.title = QFileInfo(path).fileName();
    list.path = path;
    list.category = pb::net::RuleCategory::Other;

    for (const FilterList &existing : m_lists) {
        if (existing.id == list.id)
            return false;
    }

    m_lists.append(list);
    rebuildEngine();
    return true;
}

bool FilterService::addRemoteList(const QUrl &url, const QString &title)
{
    if (!url.isValid() || url.scheme() != QLatin1String("https"))
        return false; // a list fetched over plain http could be tampered with

    FilterList list;
    list.id = QStringLiteral("remote-") + QString::number(qHash(url.toString()), 16);
    list.title = title.isEmpty() ? url.host() : title;
    list.sourceUrl = url;
    list.category = pb::net::RuleCategory::Other;

    for (const FilterList &existing : m_lists) {
        if (existing.id == list.id)
            return false;
    }

    m_lists.append(list);
    Q_EMIT listsChanged();
    updateRemoteLists();
    return true;
}

void FilterService::removeList(const QString &id)
{
    for (int i = 0; i < m_lists.size(); ++i) {
        if (m_lists.at(i).id == id && !m_lists.at(i).builtIn) {
            m_lists.removeAt(i);
            m_downloaded.remove(id);
            rebuildEngine();
            return;
        }
    }
}

void FilterService::setListEnabled(const QString &id, bool enabled)
{
    for (FilterList &list : m_lists) {
        if (list.id == id) {
            if (list.enabled == enabled)
                return;
            list.enabled = enabled;
            rebuildEngine();
            return;
        }
    }
}

void FilterService::updateRemoteLists()
{
    if (m_updating)
        return;

    QList<FilterList> remote;
    for (const FilterList &list : m_lists) {
        if (!list.sourceUrl.isEmpty())
            remote.append(list);
    }
    if (remote.isEmpty()) {
        Q_EMIT updateFinished(QString());
        return;
    }

    if (!m_network) {
        m_network = new QNetworkAccessManager(this);
        // No cookie jar and no cache: an update must not carry anything from
        // browsing, and must not leave anything behind.
        m_network->setCookieJar(nullptr);
        m_network->setCache(nullptr);
    }

    m_updating = true;
    m_pendingUpdates = static_cast<int>(remote.size());
    Q_EMIT updatingChanged();

    for (const FilterList &list : remote) {
        QNetworkRequest request(list.sourceUrl);
        // The only identity in the request is the browser name and version.
        request.setHeader(QNetworkRequest::UserAgentHeader,
                          QStringLiteral("PrivacyBrowser/%1 filter-list-updater")
                              .arg(QCoreApplication::applicationVersion()));
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                             QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                             QNetworkRequest::AlwaysNetwork);

        QNetworkReply *reply = m_network->get(request);
        const QString id = list.id;
        connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
            QString error;
            if (reply->error() != QNetworkReply::NoError) {
                error = reply->errorString();
            } else {
                const QByteArray body = reply->readAll();
                if (body.size() > kMaxListBytes)
                    error = tr("The list is too large.");
                else
                    m_downloaded.insert(id, QString::fromUtf8(body));
            }
            reply->deleteLater();

            if (--m_pendingUpdates <= 0) {
                m_updating = false;
                Q_EMIT updatingChanged();
                rebuildEngine();
                Q_EMIT updateFinished(error);
            }
        });
    }
}

QStringList FilterService::localListPaths() const
{
    QStringList paths;
    for (const FilterList &list : m_lists) {
        if (!list.builtIn && list.sourceUrl.isEmpty())
            paths.append(list.path);
    }
    return paths;
}

void FilterService::restoreLocalLists(const QStringList &paths)
{
    for (const QString &path : paths)
        addLocalList(QUrl::fromLocalFile(path));
}

} // namespace pb::network
