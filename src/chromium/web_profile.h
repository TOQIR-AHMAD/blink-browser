// The one and only browsing profile.
//
// It is created off-the-record: Qt WebEngine's default-constructed
// QWebEngineProfile keeps cookies, cache and storage in memory for the lifetime
// of the object. The browser never touches QWebEngineProfile::defaultProfile(),
// which is disk-backed.
//
// What Chromium still writes to disk regardless of this class (GPU shader
// cache, crash database, temporary files for downloads and the PDF viewer) is
// redirected into the per-session temporary directory owned by
// pb::privacy::SessionPaths and removed on exit. PLAN.md §13 forbids claiming
// more than that.

#ifndef PB_CHROMIUM_WEB_PROFILE_H
#define PB_CHROMIUM_WEB_PROFILE_H

#include "privacy/core/privacy_types.h"

#include <QtCore/QObject>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QWebEngineProfile;
class QWebEngineUrlRequestInterceptor;
QT_END_NAMESPACE

namespace pb::chromium {

struct ProfileConfig {
    pb::privacy::CookiePolicy cookiePolicy = pb::privacy::CookiePolicy::BlockThirdParty;
    pb::privacy::FingerprintProtection fingerprint = pb::privacy::FingerprintProtection::Standard;
    bool webGlEnabled = true;
    bool pdfViewerEnabled = true;
    // Absolute path of the per-session temporary directory. Empty means "let
    // Chromium choose", which is only acceptable in tests.
    QString sessionPath;
};

class WebProfile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWebEngineProfile *profile READ profile CONSTANT)

public:
    explicit WebProfile(QObject *parent = nullptr);
    ~WebProfile() override;

    QWebEngineProfile *profile() const { return m_profile; }

    void applyConfig(const ProfileConfig &config);
    ProfileConfig config() const { return m_config; }

    // Ownership stays with the caller; the interceptor must outlive the
    // profile or be cleared with nullptr first.
    void setRequestInterceptor(QWebEngineUrlRequestInterceptor *interceptor);

    // Injects (or removes) the fingerprint-protection user script according to
    // the configured level.
    void updateFingerprintProtection();

    // Idempotent. Drops cookies, cache and in-memory storage. Safe to call on
    // a profile that has already been cleared.
    void clearBrowsingData();

Q_SIGNALS:
    // Emitted when the cookie filter refused a cookie, so the privacy
    // dashboard can count it. Carries no URL.
    void cookieBlocked(bool thirdParty);

private:
    void installCookieFilter();
    void applySettings();

    QWebEngineProfile *m_profile = nullptr;
    ProfileConfig m_config;
    bool m_fingerprintScriptInstalled = false;
};

} // namespace pb::chromium

#endif // PB_CHROMIUM_WEB_PROFILE_H
