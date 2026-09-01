#include "network/request_interceptor.h"

#include "network/core/url_info.h"
#include "network/filter_service.h"
#include "privacy/core/blocking_stats.h"

#include <QtCore/QMutexLocker>
#include <QtCore/QVariantMap>

namespace pb::network {
namespace {

std::uint32_t resourceTypeOf(QWebEngineUrlRequestInfo::ResourceType type)
{
    using Resource = QWebEngineUrlRequestInfo;
    switch (type) {
    case Resource::ResourceTypeMainFrame:
        return pb::net::ResourceDocument;
    case Resource::ResourceTypeSubFrame:
        return pb::net::ResourceSubdocument;
    case Resource::ResourceTypeStylesheet:
        return pb::net::ResourceStylesheet;
    case Resource::ResourceTypeScript:
        return pb::net::ResourceScript;
    case Resource::ResourceTypeImage:
    case Resource::ResourceTypeFavicon:
        return pb::net::ResourceImage;
    case Resource::ResourceTypeFontResource:
        return pb::net::ResourceFont;
    case Resource::ResourceTypeMedia:
        return pb::net::ResourceMedia;
    case Resource::ResourceTypeXhr:
        return pb::net::ResourceXhr;
    case Resource::ResourceTypePing:
        return pb::net::ResourcePing;
    default:
        break;
    }
    return pb::net::ResourceOther;
}

} // namespace

RequestInterceptor::RequestInterceptor(FilterService *filters, pb::privacy::BlockingStats *stats,
                                       QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
    , m_filters(filters)
    , m_stats(stats)
{
}

void RequestInterceptor::setBlockingEnabled(bool trackers, bool ads)
{
    m_trackersEnabled = trackers;
    m_adsEnabled = ads;
}

void RequestInterceptor::setHttpsFirstEnabled(bool enabled)
{
    m_httpsFirstEnabled = enabled;
}

void RequestInterceptor::recordHttpsFailure(const QString &host)
{
    QMutexLocker locker(&m_mutex);
    m_httpsUpgrade.recordFailure(host.toStdString());
}

void RequestInterceptor::setAuditEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_auditEnabled = enabled;
    if (!enabled)
        m_audit.clear();
}

bool RequestInterceptor::auditEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_auditEnabled;
}

QVariantList RequestInterceptor::auditRows() const
{
    QMutexLocker locker(&m_mutex);
    QVariantList result;
    for (const pb::net::AuditRow &row : m_audit.rows()) {
        QVariantMap entry;
        entry[QStringLiteral("host")] = QString::fromStdString(row.host);
        entry[QStringLiteral("requests")] = static_cast<int>(row.requests);
        entry[QStringLiteral("blocked")] = static_cast<int>(row.blocked);
        entry[QStringLiteral("classification")] = static_cast<int>(row.classification);
        result.append(entry);
    }
    return result;
}

void RequestInterceptor::clearAudit()
{
    QMutexLocker locker(&m_mutex);
    m_audit.clear();
}

void RequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    const QUrl requestUrl = info.requestUrl();
    const QString urlText = requestUrl.toString();
    if (urlText.isEmpty())
        return;

    const pb::net::UrlInfo url = pb::net::UrlInfo::parse(urlText.toStdString());
    if (!url.isHttpFamily())
        return; // about:, blob:, data:, file: are not ours to filter

    const QUrl firstParty = info.firstPartyUrl();
    const std::string documentHost = firstParty.host().toStdString();
    const bool isMainFrame
        = info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame;

    // 1. HTTPS-first.
    if (m_httpsFirstEnabled) {
        QMutexLocker locker(&m_mutex);
        const std::string upgraded = m_httpsUpgrade.upgradedUrl(url, isMainFrame);
        if (!upgraded.empty()) {
            locker.unlock();
            if (m_stats)
                m_stats->recordHttpsUpgrade();
            info.redirect(QUrl(QString::fromStdString(upgraded)));
            return;
        }
    }

    // 2. Filtering.
    bool blocked = false;
    pb::net::RuleCategory category = pb::net::RuleCategory::Other;
    if (m_filters && (m_trackersEnabled || m_adsEnabled)) {
        pb::net::Request request;
        request.url = urlText.toStdString();
        request.documentHost = documentHost;
        request.type = resourceTypeOf(info.resourceType());
        request.thirdParty = pb::net::isThirdParty(url, documentHost);

        const pb::net::MatchResult result = m_filters->match(request);
        if (result.blocked) {
            const bool categoryEnabled
                = (result.category == pb::net::RuleCategory::Trackers && m_trackersEnabled)
                || (result.category == pb::net::RuleCategory::Ads && m_adsEnabled)
                || (result.category == pb::net::RuleCategory::Other
                    && (m_trackersEnabled || m_adsEnabled));
            if (categoryEnabled) {
                blocked = true;
                category = result.category;
            }
        }
    }

    // 3. Counting.
    if (blocked && m_stats) {
        switch (category) {
        case pb::net::RuleCategory::Ads:
            m_stats->recordAdBlocked();
            break;
        case pb::net::RuleCategory::Trackers:
            m_stats->recordTrackerBlocked();
            break;
        case pb::net::RuleCategory::Other:
            m_stats->recordThirdPartyBlocked();
            break;
        }
        Q_EMIT requestBlocked(static_cast<int>(category));
    }

    if (m_auditEnabled) {
        QMutexLocker locker(&m_mutex);
        if (m_auditEnabled) {
            const QStringList hostList = m_filters ? m_filters->filterListHosts() : QStringList();
            std::vector<std::string> filterHosts;
            filterHosts.reserve(static_cast<std::size_t>(hostList.size()));
            for (const QString &host : hostList)
                filterHosts.push_back(host.toStdString());
            m_audit.record(url.host,
                           pb::net::classifyConnection(url.host, documentHost, {}, {},
                                                       filterHosts),
                           blocked);
        }
    }

    if (blocked)
        info.block(true);
}

} // namespace pb::network
