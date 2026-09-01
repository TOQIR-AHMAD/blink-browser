// Owns the filter lists and the engine they compile into.
//
// Where lists come from (PLAN.md §20, §21):
// - Two small lists ship inside the binary. They are never fetched, so a
//   default install makes no filter-related network request at all.
// - The user may add a list from a local file, or from a URL they type. The
//   URL is theirs, not ours: there is no developer-controlled list mirror.
// - Updates happen when the user asks. Nothing is fetched on a schedule
//   unless the user turns that on, and a fetch tells the list's host only
//   what any HTTPS request tells it - an IP address and a time - because the
//   updater sends no cookies, no identifiers and no browsing information.
//
// match() is called on Chromium's network thread; the engine is swapped under
// a write lock when lists change.

#ifndef PB_NETWORK_FILTER_SERVICE_H
#define PB_NETWORK_FILTER_SERVICE_H

#include "network/core/filter_engine.h"

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QReadWriteLock>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QVariantList>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace pb::network {

struct FilterList {
    QString id;
    QString title;
    // Either a ":/filters/..." resource, or an absolute path on disk.
    QString path;
    // Empty for the built-in lists: they are not fetched from anywhere.
    QUrl sourceUrl;
    pb::net::RuleCategory category = pb::net::RuleCategory::Other;
    bool builtIn = false;
    bool enabled = true;
    int rules = 0;
    int skipped = 0;
};

class FilterService : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application as App.privacy.filters")

    Q_PROPERTY(int ruleCount READ ruleCount NOTIFY listsChanged)
    Q_PROPERTY(QVariantList lists READ listsAsVariants NOTIFY listsChanged)
    Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)

public:
    explicit FilterService(QObject *parent = nullptr);
    ~FilterService() override;

    // Thread-safe. Returns an allow result when blocking is disabled.
    pb::net::MatchResult match(const pb::net::Request &request) const;

    void setEnabled(bool trackers, bool ads);

    int ruleCount() const;
    bool updating() const { return m_updating; }
    QVariantList listsAsVariants() const;
    QStringList filterListHosts() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool addLocalList(const QUrl &fileUrl);
    Q_INVOKABLE bool addRemoteList(const QUrl &url, const QString &title);
    Q_INVOKABLE void removeList(const QString &id);
    Q_INVOKABLE void setListEnabled(const QString &id, bool enabled);

    // Refetches every list that has a source URL. Explicit user action.
    Q_INVOKABLE void updateRemoteLists();

    // Paths of user-added local lists, for the settings file.
    QStringList localListPaths() const;
    void restoreLocalLists(const QStringList &paths);

Q_SIGNALS:
    void listsChanged();
    void updatingChanged();
    // Empty message means success.
    void updateFinished(const QString &errorMessage);

private:
    void loadBuiltInLists();
    QString readList(const FilterList &list, bool *ok) const;
    void rebuildEngine();

    mutable QReadWriteLock m_lock;
    pb::net::FilterEngine m_engine;
    QList<FilterList> m_lists;
    // Downloaded list bodies, kept in memory for the session only.
    QHash<QString, QString> m_downloaded;
    QNetworkAccessManager *m_network = nullptr;
    bool m_trackersEnabled = true;
    bool m_adsEnabled = true;
    bool m_updating = false;
    int m_pendingUpdates = 0;
};

} // namespace pb::network

#endif // PB_NETWORK_FILTER_SERVICE_H
