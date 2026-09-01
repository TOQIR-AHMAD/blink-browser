#include "chromium/web_profile.h"

#include "chromium/core/user_agent.h"
#include "utils/logging.h"

#include <QtCore/QFile>
#include <QtCore/QSysInfo>
#include <QtWebEngineCore/QWebEngineCookieStore>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineCore/QWebEngineScript>
#include <QtWebEngineCore/QWebEngineScriptCollection>
#include <QtWebEngineCore/QWebEngineSettings>

namespace pb::chromium {
namespace {

constexpr char kFingerprintScriptName[] = "pb_fingerprint_protection";

UserAgentPlatform currentPlatform()
{
#if defined(Q_OS_WIN)
    return UserAgentPlatform::Windows;
#elif defined(Q_OS_MACOS)
    return UserAgentPlatform::MacOs;
#else
    return UserAgentPlatform::Linux;
#endif
}

} // namespace

WebProfile::WebProfile(QObject *parent)
    : QObject(parent)
{
    // Default-constructed: off-the-record. Never defaultProfile().
    m_profile = new QWebEngineProfile(this);

    m_profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
    m_profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    m_profile->setSpellCheckEnabled(false);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    m_profile->setPushServiceEnabled(false);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Permission grants live only as long as the session. Re-check this line
    // when moving to a newer Qt: it is the one API here that changed in 6.8.
    m_profile->setPersistentPermissionsPolicy(
        QWebEngineProfile::PersistentPermissionsPolicy::StoreInMemory);
#endif

    installCookieFilter();
    applyConfig(m_config);
}

WebProfile::~WebProfile()
{
    // The profile is a child QObject and is destroyed with this object; the
    // explicit clear keeps the ordering obvious for the session cleaner.
    clearBrowsingData();
}

void WebProfile::applyConfig(const ProfileConfig &config)
{
    m_config = config;

    if (!m_config.sessionPath.isEmpty()) {
        // Off-the-record profiles ignore the persistent storage path, but
        // Chromium still uses the download and cache paths for the few things
        // it insists on writing.
        m_profile->setDownloadPath(m_config.sessionPath + QStringLiteral("/downloads"));
        m_profile->setCachePath(m_config.sessionPath + QStringLiteral("/cache"));
        m_profile->setPersistentStoragePath(m_config.sessionPath + QStringLiteral("/storage"));
    }

    if (m_config.fingerprint != pb::privacy::FingerprintProtection::Off) {
        const std::string reduced = reducedUserAgent(
            m_profile->httpUserAgent().toStdString(), currentPlatform());
        if (!reduced.empty())
            m_profile->setHttpUserAgent(QString::fromStdString(reduced));
        else
            pb::log::write(pb::log::Level::Warning,
                           "could not derive a reduced user agent; keeping Qt's default");
    }

    if (m_config.fingerprint == pb::privacy::FingerprintProtection::Strict) {
        // A fixed accept-language removes a common entropy source. It can make
        // multilingual sites answer in English, which is why it is Strict-only.
        m_profile->setHttpAcceptLanguage(QStringLiteral("en-US,en;q=0.9"));
    }

    applySettings();
    updateFingerprintProtection();
}

void WebProfile::applySettings()
{
    QWebEngineSettings *settings = m_profile->settings();
    if (!settings)
        return;

    settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
    settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    settings->setAttribute(QWebEngineSettings::TouchIconsEnabled, false);
    settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, true);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, m_config.pdfViewerEnabled);
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, m_config.pdfViewerEnabled);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, m_config.webGlEnabled);

    // DNS prefetching resolves names for links the user never clicked.
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    // Chromium's own switch for canvas readback, which is what most canvas
    // fingerprinting depends on. Strict only: it breaks legitimate uses such as
    // image editors and chart exporters.
    settings->setAttribute(QWebEngineSettings::ReadingFromCanvasEnabled,
                           m_config.fingerprint != pb::privacy::FingerprintProtection::Strict);
#endif
}

void WebProfile::installCookieFilter()
{
    QWebEngineCookieStore *store = m_profile->cookieStore();
    if (!store)
        return;

    // The filter is called on a Chromium thread. It only reads m_config and
    // emits a signal, which Qt delivers to the main thread as a queued call.
    store->setCookieFilter([this](const QWebEngineCookieStore::FilterRequest &request) {
        switch (m_config.cookiePolicy) {
        case pb::privacy::CookiePolicy::BlockAll:
            Q_EMIT cookieBlocked(request.thirdParty);
            return false;
        case pb::privacy::CookiePolicy::BlockThirdParty:
            if (request.thirdParty) {
                Q_EMIT cookieBlocked(true);
                return false;
            }
            return true;
        case pb::privacy::CookiePolicy::AllowAll:
            return true;
        }
        return true;
    });
}

void WebProfile::setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor)
{
    m_profile->setUrlRequestInterceptor(interceptor);
}

void WebProfile::updateFingerprintProtection()
{
    QWebEngineScriptCollection *scripts = m_profile->scripts();
    if (!scripts)
        return;

    const auto existing = scripts->find(QString::fromLatin1(kFingerprintScriptName));
    for (const QWebEngineScript &script : existing)
        scripts->remove(script);
    m_fingerprintScriptInstalled = false;

    if (m_config.fingerprint != pb::privacy::FingerprintProtection::Strict)
        return;

    QFile source(QStringLiteral(":/scripts/fingerprint_protection.js"));
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text)) {
        pb::log::write(pb::log::Level::Error,
                       "fingerprint protection script missing from the application resources");
        return;
    }

    QWebEngineScript script;
    script.setName(QString::fromLatin1(kFingerprintScriptName));
    script.setSourceCode(QString::fromUtf8(source.readAll()));
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld); // must see the page's own objects
    script.setRunsOnSubFrames(true);
    scripts->insert(script);
    m_fingerprintScriptInstalled = true;
}

void WebProfile::clearBrowsingData()
{
    if (!m_profile)
        return;
    if (QWebEngineCookieStore *store = m_profile->cookieStore())
        store->deleteAllCookies();
    m_profile->clearHttpCache();
    m_profile->clearAllVisitedLinks();
}

} // namespace pb::chromium
