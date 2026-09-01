// The settings object the UI binds to.
//
// It wraps pb::settings::BrowserSettings (the Qt-free definition of what a
// setting is) and adds two things: change notification for QML, and the opt-in
// on-disk store.
//
// Persistence rules:
// - Nothing is written unless the user turns on "Remember settings".
// - Only settings are ever written. No history, no URLs, no cookies, no
//   identifiers - the file is a flat map of the keys in
//   BrowserSettings::toMap().
// - Turning the option off deletes the file rather than merely ignoring it.

#ifndef PB_SETTINGS_SETTINGS_CONTROLLER_H
#define PB_SETTINGS_SETTINGS_CONTROLLER_H

#include "settings/core/browser_settings.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantList>
#include <QtQml/qqmlregistration.h>

namespace pb::settings {

class SettingsController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application as App.settings")

    Q_PROPERTY(bool trackerBlocking READ trackerBlocking WRITE setTrackerBlocking NOTIFY changed)
    Q_PROPERTY(bool adBlocking READ adBlocking WRITE setAdBlocking NOTIFY changed)
    Q_PROPERTY(int cookiePolicy READ cookiePolicy WRITE setCookiePolicy NOTIFY changed)
    Q_PROPERTY(int fingerprintProtection READ fingerprintProtection WRITE setFingerprintProtection
                   NOTIFY changed)
    Q_PROPERTY(bool httpsFirst READ httpsFirst WRITE setHttpsFirst NOTIFY changed)
    Q_PROPERTY(bool searchSuggestions READ searchSuggestions WRITE setSearchSuggestions NOTIFY
                   changed)
    Q_PROPERTY(bool rememberSettings READ rememberSettings WRITE setRememberSettings NOTIFY changed)

    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY changed)
    Q_PROPERTY(bool glassEffects READ glassEffects WRITE setGlassEffects NOTIFY changed)
    Q_PROPERTY(bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY changed)
    Q_PROPERTY(qreal textScale READ textScale WRITE setTextScale NOTIFY changed)

    Q_PROPERTY(QString searchEngineId READ searchEngineId WRITE setSearchEngineId NOTIFY changed)
    Q_PROPERTY(QString customSearchTemplate READ customSearchTemplate WRITE
                   setCustomSearchTemplate NOTIFY changed)

    Q_PROPERTY(int dnsMode READ dnsMode WRITE setDnsMode NOTIFY changed)
    Q_PROPERTY(QString dohTemplate READ dohTemplate WRITE setDohTemplate NOTIFY changed)
    Q_PROPERTY(int proxyMode READ proxyMode WRITE setProxyMode NOTIFY changed)
    Q_PROPERTY(QString proxyHost READ proxyHost WRITE setProxyHost NOTIFY changed)
    Q_PROPERTY(int proxyPort READ proxyPort WRITE setProxyPort NOTIFY changed)

    Q_PROPERTY(bool askWhereToSave READ askWhereToSave WRITE setAskWhereToSave NOTIFY changed)
    Q_PROPERTY(QString downloadDirectory READ downloadDirectory WRITE setDownloadDirectory NOTIFY
                   changed)

    Q_PROPERTY(bool checkFilterListUpdatesOnStart READ checkFilterListUpdatesOnStart WRITE
                   setCheckFilterListUpdatesOnStart NOTIFY changed)

    Q_PROPERTY(QString storagePath READ storagePath NOTIFY changed)
    Q_PROPERTY(bool storedOnDisk READ storedOnDisk NOTIFY changed)

public:
    // Mirrors of the core enums so QML can name them.
    enum CookiePolicyValue { AllowAllCookies, BlockThirdPartyCookies, BlockAllCookies };
    Q_ENUM(CookiePolicyValue)
    enum FingerprintValue { FingerprintOff, FingerprintStandard, FingerprintStrict };
    Q_ENUM(FingerprintValue)
    enum ThemeValue { SystemTheme, LightTheme, DarkTheme };
    Q_ENUM(ThemeValue)
    enum DnsValue { SystemDns, SecureDns };
    Q_ENUM(DnsValue)
    enum ProxyValue { SystemProxy, NoProxy, HttpProxy, Socks5Proxy };
    Q_ENUM(ProxyValue)

    explicit SettingsController(QObject *parent = nullptr);

    // Reads the stored file if one exists. Safe to call before QGuiApplication
    // exists, which is what lets the Chromium switches honour the user's DNS
    // and proxy choice on the very first launch of a session.
    static BrowserSettings loadStoredSettings();

    const BrowserSettings &settings() const { return m_settings; }

    bool trackerBlocking() const { return m_settings.trackerBlocking; }
    bool adBlocking() const { return m_settings.adBlocking; }
    int cookiePolicy() const;
    int fingerprintProtection() const;
    bool httpsFirst() const { return m_settings.httpsFirst; }
    bool searchSuggestions() const { return m_settings.searchSuggestions; }
    bool rememberSettings() const { return m_settings.rememberSettings; }
    int theme() const;
    bool glassEffects() const { return m_settings.glassEffects; }
    bool reducedMotion() const { return m_settings.reducedMotion; }
    qreal textScale() const { return m_settings.textScale; }
    QString searchEngineId() const;
    QString customSearchTemplate() const;
    int dnsMode() const;
    QString dohTemplate() const;
    int proxyMode() const;
    QString proxyHost() const;
    int proxyPort() const { return m_settings.proxyPort; }
    bool askWhereToSave() const { return m_settings.askWhereToSave; }
    QString downloadDirectory() const;
    bool checkFilterListUpdatesOnStart() const
    {
        return m_settings.checkFilterListUpdatesOnStart;
    }
    QString storagePath() const;
    bool storedOnDisk() const;

    void setTrackerBlocking(bool value);
    void setAdBlocking(bool value);
    void setCookiePolicy(int value);
    void setFingerprintProtection(int value);
    void setHttpsFirst(bool value);
    void setSearchSuggestions(bool value);
    void setRememberSettings(bool value);
    void setTheme(int value);
    void setGlassEffects(bool value);
    void setReducedMotion(bool value);
    void setTextScale(qreal value);
    void setSearchEngineId(const QString &value);
    void setCustomSearchTemplate(const QString &value);
    void setDnsMode(int value);
    void setDohTemplate(const QString &value);
    void setProxyMode(int value);
    void setProxyHost(const QString &value);
    void setProxyPort(int value);
    void setAskWhereToSave(bool value);
    void setDownloadDirectory(const QString &value);
    void setCheckFilterListUpdatesOnStart(bool value);

    // Lists for the settings UI.
    Q_INVOKABLE QVariantList searchEngines() const;
    Q_INVOKABLE QVariantList privacyGuarantees() const;

    Q_INVOKABLE void resetToDefaults();
    // Removes the settings file, whatever the current option says.
    Q_INVOKABLE bool deleteStoredSettings();

Q_SIGNALS:
    // One signal for every property: subsystems reapply their configuration
    // wholesale rather than tracking individual keys.
    void changed();

private:
    void commit();

    BrowserSettings m_settings;
};

} // namespace pb::settings

#endif // PB_SETTINGS_SETTINGS_CONTROLLER_H
