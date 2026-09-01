// Owns everything with a lifetime equal to the running browser.
//
// The object graph is built here once, in one place, so that the shutdown order
// is explicit: windows and tabs first, then the profile, then the session
// directory (PLAN.md §14).

#ifndef PB_APP_APPLICATION_H
#define PB_APP_APPLICATION_H

#include "privacy/core/blocking_stats.h"
#include "privacy/core/session_paths.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE
class QQmlApplicationEngine;
class QWebEngineProfile;
QT_END_NAMESPACE

namespace pb::chromium {
class WebProfile;
}
namespace pb::browser {
class BrowserController;
}
namespace pb::downloads {
class DownloadManager;
}
namespace pb::network {
class FilterService;
class RequestInterceptor;
}
namespace pb::permissions {
class PermissionManager;
}
namespace pb::privacy {
class PrivacyManager;
}
namespace pb::settings {
class SettingsController;
}

namespace pb::app {

class BrowserApplication : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided to QML as App")

    Q_PROPERTY(QWebEngineProfile *profile READ webEngineProfile CONSTANT)
    Q_PROPERTY(pb::settings::SettingsController *settings READ settings CONSTANT)
    Q_PROPERTY(pb::browser::BrowserController *browser READ browser CONSTANT)
    Q_PROPERTY(pb::privacy::PrivacyManager *privacy READ privacy CONSTANT)
    Q_PROPERTY(pb::permissions::PermissionManager *permissions READ permissions CONSTANT)
    Q_PROPERTY(pb::downloads::DownloadManager *downloads READ downloads CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString chromiumVersion READ chromiumVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(bool firstRun READ firstRun CONSTANT)

public:
    // Must run before QtWebEngineQuick::initialize() and before QGuiApplication
    // exists: it is what puts the privacy switches on Chromium's command line.
    static void prepareEnvironment();

    explicit BrowserApplication(QObject *parent = nullptr);
    ~BrowserApplication() override;

    void registerWithEngine(QQmlApplicationEngine &engine);

    // Idempotent. Runs the shutdown sequence and reports whether the session
    // directory is gone.
    bool shutdown();

    QWebEngineProfile *webEngineProfile() const;
    pb::settings::SettingsController *settings() const { return m_settings; }
    pb::browser::BrowserController *browser() const { return m_browser; }
    pb::privacy::PrivacyManager *privacy() const { return m_privacy; }
    pb::permissions::PermissionManager *permissions() const { return m_permissions; }
    pb::downloads::DownloadManager *downloads() const { return m_downloads; }

    QString version() const;
    QString chromiumVersion() const;
    QString qtVersion() const;
    // True when no settings file exists yet, which is what the welcome screen
    // keys off. It is not a "have you been here before" flag: with settings
    // persistence off (the default) every launch is a first run.
    bool firstRun() const { return m_firstRun; }

public Q_SLOTS:
    // Re-applies the current settings to the profile, the interceptor and the
    // network proxy. Connected to SettingsController::changed().
    void applyConfiguration();

private:
    pb::privacy::SessionPaths m_sessionPaths;
    pb::privacy::BlockingStats m_stats;
    pb::settings::SettingsController *m_settings = nullptr;
    pb::chromium::WebProfile *m_webProfile = nullptr;
    pb::network::FilterService *m_filters = nullptr;
    pb::network::RequestInterceptor *m_interceptor = nullptr;
    pb::privacy::PrivacyManager *m_privacy = nullptr;
    pb::permissions::PermissionManager *m_permissions = nullptr;
    pb::downloads::DownloadManager *m_downloads = nullptr;
    pb::browser::BrowserController *m_browser = nullptr;
    bool m_firstRun = true;
    bool m_shutdownDone = false;
};

} // namespace pb::app

#endif // PB_APP_APPLICATION_H
