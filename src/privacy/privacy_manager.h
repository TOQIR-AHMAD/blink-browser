// The privacy subsystem's face to the rest of the browser.
//
// It owns the blocking counters, the filter service and the shutdown sequence,
// and it is what the privacy dashboard binds to. It deliberately depends on
// nothing in the UI: the UI observes it (PLAN.md §12).

#ifndef PB_PRIVACY_PRIVACY_MANAGER_H
#define PB_PRIVACY_PRIVACY_MANAGER_H

#include "privacy/core/blocking_stats.h"
#include "privacy/core/cleanup_sequence.h"
#include "privacy/core/session_paths.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantList>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace pb::chromium {
class WebProfile;
}
namespace pb::network {
class FilterService;
class RequestInterceptor;
}
namespace pb::settings {
class SettingsController;
}

namespace pb::privacy {

class PrivacyManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application as App.privacy")

    Q_PROPERTY(int trackersBlocked READ trackersBlocked NOTIFY statsChanged)
    Q_PROPERTY(int adsBlocked READ adsBlocked NOTIFY statsChanged)
    Q_PROPERTY(int cookiesBlocked READ cookiesBlocked NOTIFY statsChanged)
    Q_PROPERTY(int thirdPartyBlocked READ thirdPartyBlocked NOTIFY statsChanged)
    Q_PROPERTY(int httpsUpgrades READ httpsUpgrades NOTIFY statsChanged)
    Q_PROPERTY(int totalBlocked READ totalBlocked NOTIFY statsChanged)
    Q_PROPERTY(pb::network::FilterService *filters READ filters CONSTANT)
    Q_PROPERTY(QString sessionDirectory READ sessionDirectory CONSTANT)
    Q_PROPERTY(bool auditAvailable READ auditAvailable CONSTANT)
    Q_PROPERTY(bool auditEnabled READ auditEnabled WRITE setAuditEnabled NOTIFY auditChanged)

public:
    PrivacyManager(pb::chromium::WebProfile *profile, pb::network::FilterService *filters,
                   pb::network::RequestInterceptor *interceptor,
                   pb::settings::SettingsController *settings, BlockingStats *stats,
                   SessionPaths *sessionPaths, QObject *parent = nullptr);

    int trackersBlocked() const;
    int adsBlocked() const;
    int cookiesBlocked() const;
    int thirdPartyBlocked() const;
    int httpsUpgrades() const;
    int totalBlocked() const;

    pb::network::FilterService *filters() const { return m_filters; }
    QString sessionDirectory() const;

    bool auditAvailable() const;
    bool auditEnabled() const;
    void setAuditEnabled(bool enabled);

    // Clears cookies, cache and in-memory storage without restarting.
    Q_INVOKABLE void clearBrowsingDataNow();
    Q_INVOKABLE void resetStatistics();
    Q_INVOKABLE QVariantList auditRows() const;
    Q_INVOKABLE void clearAudit();

    // What the browser keeps and where, for the privacy page. Each row is
    // {name, location, lifetime}.
    Q_INVOKABLE QVariantList storageSummary() const;

    // Called by the window when an https attempt failed, so the host is not
    // upgraded again this session.
    Q_INVOKABLE void recordHttpsFailure(const QString &host);

    // Re-applies the current settings to the profile and the interceptor.
    void applySettings();

    // Runs the shutdown sequence. Idempotent; safe to call more than once.
    CleanupOutcome runCleanup();

Q_SIGNALS:
    void statsChanged();
    void auditChanged();
    void cleanupFailed(const QString &stepName);

private Q_SLOTS:
    void scheduleStatsUpdate();

private:
    void buildCleanupSequence();

    pb::chromium::WebProfile *m_profile = nullptr;
    pb::network::FilterService *m_filters = nullptr;
    pb::network::RequestInterceptor *m_interceptor = nullptr;
    pb::settings::SettingsController *m_settings = nullptr;
    SessionPaths *m_sessionPaths = nullptr;

    BlockingStats *m_stats = nullptr;
    CleanupSequence m_cleanup;
    QTimer *m_statsTimer = nullptr;
};

} // namespace pb::privacy

#endif // PB_PRIVACY_PRIVACY_MANAGER_H
