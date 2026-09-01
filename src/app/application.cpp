#include "app/application.h"

#include "browser/browser_controller.h"
#include "browser/window_controller.h"
#include "chromium/core/chromium_flags.h"
#include "chromium/web_profile.h"
#include "downloads/download_manager.h"
#include "network/filter_service.h"
#include "network/request_interceptor.h"
#include "permissions/permission_manager.h"
#include "privacy/privacy_manager.h"
#include "settings/settings_controller.h"
#include "tabs/tab_model.h"
#include "utils/logging.h"

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QSysInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QtGlobal>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/qtwebenginecoreglobal.h>

namespace pb::app {
namespace {

pb::chromium::FlagsConfig flagsFor(const pb::settings::BrowserSettings &settings)
{
    pb::chromium::FlagsConfig config;
    if (settings.dnsMode == pb::privacy::SecureDnsMode::SecureDoh) {
        config.dnsMode = pb::chromium::DnsMode::SecureDoh;
        config.dohTemplate = settings.dohTemplate;
    }
    if (settings.fingerprintProtection == pb::privacy::FingerprintProtection::Off)
        config.webRtcPolicy = pb::chromium::WebRtcPolicy::Default;
    return config;
}

} // namespace

void BrowserApplication::prepareEnvironment()
{
    // The stored settings, if the user opted into keeping them, decide the DNS
    // and WebRTC switches - and those have to be on Chromium's command line
    // before it starts.
    const pb::settings::BrowserSettings settings
        = pb::settings::SettingsController::loadStoredSettings();
    const std::string flags = pb::chromium::buildFlags(flagsFor(settings));
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArray::fromStdString(flags));
}

BrowserApplication::BrowserApplication(QObject *parent)
    : QObject(parent)
{
    m_sessionPaths = pb::privacy::SessionPaths::createTemporary(
        QDir::tempPath().toStdString());
    const QString sessionPath
        = QString::fromStdString(m_sessionPaths.root().generic_string());
    pb::log::write(pb::log::Level::Info, "session directory created");

    m_settings = new pb::settings::SettingsController(this);
    m_firstRun = !m_settings->storedOnDisk();

    m_webProfile = new pb::chromium::WebProfile(this);
    m_filters = new pb::network::FilterService(this);
    m_interceptor = new pb::network::RequestInterceptor(m_filters, &m_stats, this);
    m_webProfile->setRequestInterceptor(m_interceptor);

    m_privacy = new pb::privacy::PrivacyManager(m_webProfile, m_filters, m_interceptor,
                                                m_settings, &m_stats, &m_sessionPaths, this);
    m_permissions = new pb::permissions::PermissionManager(&m_stats, this);
    m_downloads = new pb::downloads::DownloadManager(m_settings, sessionPath, this);
    m_browser = new pb::browser::BrowserController(m_settings, this);

    if (QWebEngineProfile *profile = m_webProfile->profile()) {
        connect(profile, &QWebEngineProfile::downloadRequested, m_downloads,
                &pb::downloads::DownloadManager::handleDownloadRequested);
    }
    connect(m_settings, &pb::settings::SettingsController::changed, this,
            &BrowserApplication::applyConfiguration);

    m_filters->restoreLocalLists(QStringList());
    applyConfiguration();
}

BrowserApplication::~BrowserApplication()
{
    shutdown();
}

void BrowserApplication::registerWithEngine(QQmlApplicationEngine &engine)
{
    engine.rootContext()->setContextProperty(QStringLiteral("App"), this);
}

QWebEngineProfile *BrowserApplication::webEngineProfile() const
{
    return m_webProfile ? m_webProfile->profile() : nullptr;
}

QString BrowserApplication::version() const
{
    return QStringLiteral(PB_VERSION);
}

QString BrowserApplication::chromiumVersion() const
{
    return QString::fromLatin1(qWebEngineChromiumVersion());
}

QString BrowserApplication::qtVersion() const
{
    return QString::fromLatin1(qVersion());
}

QString BrowserApplication::technicalReport(const QString &context) const
{
    // Everything in here is about the software, not about the person using it.
    QStringList lines;
    lines << QStringLiteral("Privacy Browser technical report");
    lines << QStringLiteral("Generated: %1")
                 .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    lines << QStringLiteral("Application: %1").arg(version());
    lines << QStringLiteral("Chromium: %1").arg(chromiumVersion());
    lines << QStringLiteral("Qt: %1").arg(qtVersion());
    lines << QStringLiteral("Platform: %1 (%2)")
                 .arg(QSysInfo::prettyProductName(), QSysInfo::currentCpuArchitecture());
    lines << QStringLiteral("Kernel: %1 %2")
                 .arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
    if (!context.isEmpty())
        lines << QStringLiteral("Context: %1").arg(context);
    lines << QString();
    lines << QStringLiteral("No addresses, page content, cookies or identifiers are included,");
    lines << QStringLiteral("and nothing here has been sent anywhere.");
    return lines.join(QLatin1Char('\n'));
}

bool BrowserApplication::exportReport(const QUrl &target, const QString &text) const
{
    const QString path = target.isLocalFile() ? target.toLocalFile() : target.toString();
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        pb::log::write(pb::log::Level::Error, "could not write the technical report");
        return false;
    }
    file.write(text.toUtf8());
    return true;
}

void BrowserApplication::applyConfiguration()
{
    if (!m_settings)
        return;

    const pb::settings::BrowserSettings &values = m_settings->settings();

    // The profile, filters and interceptor are re-applied by the privacy
    // manager, which owns that policy. Everything left is process-level.
    if (m_privacy)
        m_privacy->applySettings();

    // Chromium reads DNS and WebRTC switches only at start-up; a change to
    // those takes effect on the next launch, which the settings page says.
    Q_UNUSED(values)
}

bool BrowserApplication::shutdown()
{
    if (m_shutdownDone)
        return !m_sessionPaths.exists();
    m_shutdownDone = true;

    // Order matters: windows and tabs, then downloads, then the profile, then
    // the session directory (PLAN.md §14).
    if (m_browser)
        m_browser->shutdown();
    if (m_downloads)
        m_downloads->shutdown();
    if (m_permissions)
        m_permissions->clear();

    if (m_webProfile)
        m_webProfile->setRequestInterceptor(nullptr);

    bool ok = true;
    if (m_privacy) {
        const pb::privacy::CleanupOutcome outcome = m_privacy->runCleanup();
        ok = outcome.completed();
        if (!ok) {
            pb::log::write(pb::log::Level::Error,
                           "session cleanup incomplete; see the steps above");
        }
    } else {
        ok = m_sessionPaths.cleanup().completed;
    }

    if (ok)
        pb::log::write(pb::log::Level::Info, "session cleanup complete");
    return ok;
}

} // namespace pb::app
