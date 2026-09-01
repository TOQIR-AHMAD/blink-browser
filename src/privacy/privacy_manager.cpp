#include "privacy/privacy_manager.h"

#include "chromium/web_profile.h"
#include "network/filter_service.h"
#include "network/request_interceptor.h"
#include "settings/settings_controller.h"
#include "utils/logging.h"

#include <QtCore/QTimer>
#include <QtCore/QVariantMap>

namespace pb::privacy {
namespace {
constexpr int kStatsCoalesceMs = 250;
}

PrivacyManager::PrivacyManager(pb::chromium::WebProfile *profile,
                               pb::network::FilterService *filters,
                               pb::network::RequestInterceptor *interceptor,
                               pb::settings::SettingsController *settings, BlockingStats *stats,
                               SessionPaths *sessionPaths, QObject *parent)
    : QObject(parent)
    , m_profile(profile)
    , m_filters(filters)
    , m_interceptor(interceptor)
    , m_settings(settings)
    , m_sessionPaths(sessionPaths)
    , m_stats(stats)
{
    // Blocked requests arrive from the network thread in bursts; the dashboard
    // does not need to redraw for each one.
    m_statsTimer = new QTimer(this);
    m_statsTimer->setSingleShot(true);
    m_statsTimer->setInterval(kStatsCoalesceMs);
    connect(m_statsTimer, &QTimer::timeout, this, &PrivacyManager::statsChanged);

    if (m_interceptor) {
        connect(m_interceptor, &pb::network::RequestInterceptor::requestBlocked, this,
                &PrivacyManager::scheduleStatsUpdate);
    }
    if (m_profile) {
        connect(m_profile, &pb::chromium::WebProfile::cookieBlocked, this, [this](bool) {
            m_stats->recordCookieBlocked();
            scheduleStatsUpdate();
        });
    }
    if (m_settings) {
        connect(m_settings, &pb::settings::SettingsController::changed, this,
                &PrivacyManager::applySettings);
    }

    buildCleanupSequence();
    applySettings();
}

void PrivacyManager::scheduleStatsUpdate()
{
    if (!m_statsTimer->isActive())
        m_statsTimer->start();
}

int PrivacyManager::trackersBlocked() const
{
    return static_cast<int>(m_stats->snapshot().trackersBlocked);
}

int PrivacyManager::adsBlocked() const
{
    return static_cast<int>(m_stats->snapshot().adsBlocked);
}

int PrivacyManager::cookiesBlocked() const
{
    return static_cast<int>(m_stats->snapshot().cookiesBlocked);
}

int PrivacyManager::thirdPartyBlocked() const
{
    return static_cast<int>(m_stats->snapshot().thirdPartyRequestsBlocked);
}

int PrivacyManager::httpsUpgrades() const
{
    return static_cast<int>(m_stats->snapshot().httpsUpgrades);
}

int PrivacyManager::totalBlocked() const
{
    return static_cast<int>(m_stats->snapshot().total());
}

QString PrivacyManager::sessionDirectory() const
{
    if (!m_sessionPaths || !m_sessionPaths->isValid())
        return {};
    return QString::fromStdString(m_sessionPaths->root().generic_string());
}

bool PrivacyManager::auditAvailable() const
{
#ifdef PB_NETWORK_AUDIT
    return true;
#else
    return false;
#endif
}

bool PrivacyManager::auditEnabled() const
{
    return m_interceptor && m_interceptor->auditEnabled();
}

void PrivacyManager::setAuditEnabled(bool enabled)
{
    if (!m_interceptor || !auditAvailable())
        return;
    if (m_interceptor->auditEnabled() == enabled)
        return;
    m_interceptor->setAuditEnabled(enabled);
    Q_EMIT auditChanged();
}

QVariantList PrivacyManager::auditRows() const
{
    return m_interceptor ? m_interceptor->auditRows() : QVariantList();
}

void PrivacyManager::clearAudit()
{
    if (m_interceptor)
        m_interceptor->clearAudit();
    Q_EMIT auditChanged();
}

void PrivacyManager::clearBrowsingDataNow()
{
    if (m_profile)
        m_profile->clearBrowsingData();
    pb::log::write(pb::log::Level::Info, "browsing data cleared on request");
}

void PrivacyManager::resetStatistics()
{
    m_stats->reset();
    Q_EMIT statsChanged();
}

void PrivacyManager::recordHttpsFailure(const QString &host)
{
    if (m_interceptor)
        m_interceptor->recordHttpsFailure(host);
}

QVariantList PrivacyManager::storageSummary() const
{
    const auto row = [](const QString &name, const QString &location, const QString &lifetime) {
        QVariantMap entry;
        entry[QStringLiteral("name")] = name;
        entry[QStringLiteral("location")] = location;
        entry[QStringLiteral("lifetime")] = lifetime;
        return QVariant(entry);
    };

    QVariantList summary;
    summary.append(row(tr("History"), tr("Memory"), tr("Until the browser closes")));
    summary.append(row(tr("Cookies"), tr("Memory"), tr("Until the browser closes")));
    summary.append(row(tr("HTTP cache"), tr("Memory"), tr("Until the browser closes")));
    summary.append(row(tr("Local storage, IndexedDB, service workers"), tr("Memory"),
                       tr("Until the browser closes")));
    summary.append(row(tr("Permission answers"), tr("Memory"), tr("Until the browser closes")));
    summary.append(row(tr("Blocking statistics"), tr("Memory"), tr("Until the browser closes")));
    summary.append(row(tr("Passwords, autofill, bookmarks"), tr("Not stored"), tr("Never kept")));
    summary.append(row(tr("GPU shader cache and Chromium temporary files"),
                       tr("Session folder in the system temporary directory"),
                       tr("Deleted when the browser closes")));
    summary.append(row(tr("Downloaded files"), tr("Where you saved them"),
                       tr("Yours - the browser does not delete them")));
    summary.append(row(tr("Settings"),
                       m_settings && m_settings->rememberSettings()
                           ? tr("Configuration file you opted into")
                           : tr("Memory"),
                       m_settings && m_settings->rememberSettings()
                           ? tr("Until you turn the option off")
                           : tr("Until the browser closes")));
    return summary;
}

void PrivacyManager::applySettings()
{
    if (!m_settings)
        return;

    const pb::settings::BrowserSettings &values = m_settings->settings();

    if (m_filters)
        m_filters->setEnabled(values.trackerBlocking, values.adBlocking);
    if (m_interceptor) {
        m_interceptor->setBlockingEnabled(values.trackerBlocking, values.adBlocking);
        m_interceptor->setHttpsFirstEnabled(values.httpsFirst);
    }
    if (m_profile) {
        pb::chromium::ProfileConfig config = m_profile->config();
        config.cookiePolicy = values.cookiePolicy;
        config.fingerprint = values.fingerprintProtection;
        m_profile->applyConfig(config);
    }
}

void PrivacyManager::buildCleanupSequence()
{
    m_cleanup.addStep("clear browsing data", [this](std::string *detail) {
        if (!m_profile) {
            *detail = "no profile";
            return true;
        }
        m_profile->clearBrowsingData();
        return true;
    });

    m_cleanup.addStep("clear blocking statistics", [this](std::string *) {
        m_stats->reset();
        return true;
    });

    m_cleanup.addStep("clear network audit", [this](std::string *) {
        if (m_interceptor)
            m_interceptor->clearAudit();
        return true;
    });

    m_cleanup.addStep("delete session directory", [this](std::string *detail) {
        if (!m_sessionPaths || !m_sessionPaths->isValid())
            return true;
        const CleanupResult result = m_sessionPaths->cleanup();
        if (!result.completed) {
            *detail = result.error;
            return false;
        }
        return true;
    });
}

CleanupOutcome PrivacyManager::runCleanup()
{
    CleanupOutcome outcome = m_cleanup.run();
    for (const CleanupStepResult &step : outcome.steps) {
        if (!step.succeeded)
            Q_EMIT cleanupFailed(QString::fromStdString(step.name));
    }
    return outcome;
}

} // namespace pb::privacy
