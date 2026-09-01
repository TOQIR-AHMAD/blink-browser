#include "app/application.h"

#include "chromium/core/chromium_flags.h"
#include "chromium/web_profile.h"
#include "utils/logging.h"

#include <QtCore/QDir>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>

namespace pb::app {

void BrowserApplication::prepareEnvironment()
{
    const pb::chromium::FlagsConfig config; // privacy-preserving defaults
    const std::string flags = pb::chromium::buildFlags(config);
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArray::fromStdString(flags));
}

BrowserApplication::BrowserApplication(QObject *parent)
    : QObject(parent)
{
    m_sessionPaths = pb::privacy::SessionPaths::createTemporary(
        QDir::tempPath().toStdString());
    pb::log::write(pb::log::Level::Info, "session directory created");

    m_webProfile = new pb::chromium::WebProfile(this);

    pb::chromium::ProfileConfig profileConfig;
    profileConfig.sessionPath
        = QString::fromStdString(m_sessionPaths.root().generic_string());
    m_webProfile->applyConfig(profileConfig);
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

bool BrowserApplication::shutdown()
{
    if (m_shutdownDone)
        return !m_sessionPaths.exists();
    m_shutdownDone = true;

    if (m_webProfile) {
        m_webProfile->clearBrowsingData();
        delete m_webProfile;
        m_webProfile = nullptr;
    }

    const pb::privacy::CleanupResult result = m_sessionPaths.cleanup();
    if (!result.completed) {
        pb::log::write(pb::log::Level::Error, "session cleanup incomplete");
        return false;
    }

    pb::log::write(pb::log::Level::Info, "session cleanup complete");
    return true;
}

} // namespace pb::app
