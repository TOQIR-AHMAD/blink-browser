#include "settings/settings_controller.h"

#include "settings/core/search_engines.h"
#include "utils/logging.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSaveFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QVariantMap>

namespace pb::settings {
namespace {

using pb::privacy::CookiePolicy;
using pb::privacy::FingerprintProtection;
using pb::privacy::ProxyMode;
using pb::privacy::SecureDnsMode;

QString settingsFilePath()
{
    const QString directory
        = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (directory.isEmpty())
        return {};
    return directory + QStringLiteral("/settings.json");
}

} // namespace

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
    , m_settings(loadStoredSettings())
{
}

BrowserSettings SettingsController::loadStoredSettings()
{
    BrowserSettings settings;

    const QString path = settingsFilePath();
    if (path.isEmpty())
        return settings;

    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return settings;

    QJsonParseError error{};
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        pb::log::write(pb::log::Level::Warning,
                       "settings file is not valid JSON; using defaults");
        return settings;
    }

    std::map<std::string, std::string> values;
    const QJsonObject object = document.object();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.value().isString())
            values[it.key().toStdString()] = it.value().toString().toStdString();
        else if (it.value().isBool())
            values[it.key().toStdString()] = it.value().toBool() ? "true" : "false";
        else if (it.value().isDouble())
            values[it.key().toStdString()] = std::to_string(it.value().toDouble());
    }

    settings = BrowserSettings::fromMap(values);
    // A stored file only exists because the option was on; keep it on.
    settings.rememberSettings = true;
    return settings;
}

void SettingsController::commit()
{
    m_settings.sanitize();

    const QString path = settingsFilePath();
    if (!m_settings.rememberSettings) {
        if (!path.isEmpty() && QFile::exists(path))
            deleteStoredSettings();
        Q_EMIT changed();
        return;
    }

    if (path.isEmpty()) {
        pb::log::write(pb::log::Level::Warning, "no writable config location for settings");
        Q_EMIT changed();
        return;
    }

    QDir().mkpath(QFileInfo(path).absolutePath());

    QJsonObject object;
    for (const auto &entry : m_settings.toMap())
        object.insert(QString::fromStdString(entry.first), QString::fromStdString(entry.second));

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        pb::log::write(pb::log::Level::Error, "could not open the settings file for writing");
        Q_EMIT changed();
        return;
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    if (!file.commit())
        pb::log::write(pb::log::Level::Error, "could not write the settings file");

    Q_EMIT changed();
}

bool SettingsController::deleteStoredSettings()
{
    const QString path = settingsFilePath();
    if (path.isEmpty() || !QFile::exists(path))
        return true;
    const bool removed = QFile::remove(path);
    if (!removed)
        pb::log::write(pb::log::Level::Error, "could not delete the settings file");
    return removed;
}

QString SettingsController::storagePath() const
{
    return settingsFilePath();
}

bool SettingsController::storedOnDisk() const
{
    const QString path = settingsFilePath();
    return !path.isEmpty() && QFile::exists(path);
}

int SettingsController::cookiePolicy() const
{
    switch (m_settings.cookiePolicy) {
    case CookiePolicy::AllowAll:
        return AllowAllCookies;
    case CookiePolicy::BlockThirdParty:
        return BlockThirdPartyCookies;
    case CookiePolicy::BlockAll:
        return BlockAllCookies;
    }
    return BlockThirdPartyCookies;
}

void SettingsController::setCookiePolicy(int value)
{
    CookiePolicy policy = CookiePolicy::BlockThirdParty;
    if (value == AllowAllCookies)
        policy = CookiePolicy::AllowAll;
    else if (value == BlockAllCookies)
        policy = CookiePolicy::BlockAll;
    if (policy == m_settings.cookiePolicy)
        return;
    m_settings.cookiePolicy = policy;
    commit();
}

int SettingsController::fingerprintProtection() const
{
    switch (m_settings.fingerprintProtection) {
    case FingerprintProtection::Off:
        return FingerprintOff;
    case FingerprintProtection::Standard:
        return FingerprintStandard;
    case FingerprintProtection::Strict:
        return FingerprintStrict;
    }
    return FingerprintStandard;
}

void SettingsController::setFingerprintProtection(int value)
{
    FingerprintProtection level = FingerprintProtection::Standard;
    if (value == FingerprintOff)
        level = FingerprintProtection::Off;
    else if (value == FingerprintStrict)
        level = FingerprintProtection::Strict;
    if (level == m_settings.fingerprintProtection)
        return;
    m_settings.fingerprintProtection = level;
    commit();
}

int SettingsController::theme() const
{
    switch (m_settings.theme) {
    case ThemeMode::System:
        return SystemTheme;
    case ThemeMode::Light:
        return LightTheme;
    case ThemeMode::Dark:
        return DarkTheme;
    }
    return SystemTheme;
}

void SettingsController::setTheme(int value)
{
    ThemeMode mode = ThemeMode::System;
    if (value == LightTheme)
        mode = ThemeMode::Light;
    else if (value == DarkTheme)
        mode = ThemeMode::Dark;
    if (mode == m_settings.theme)
        return;
    m_settings.theme = mode;
    commit();
}

int SettingsController::dnsMode() const
{
    return m_settings.dnsMode == SecureDnsMode::SecureDoh ? SecureDns : SystemDns;
}

void SettingsController::setDnsMode(int value)
{
    const SecureDnsMode mode
        = value == SecureDns ? SecureDnsMode::SecureDoh : SecureDnsMode::System;
    if (mode == m_settings.dnsMode)
        return;
    m_settings.dnsMode = mode;
    commit();
}

int SettingsController::proxyMode() const
{
    switch (m_settings.proxyMode) {
    case ProxyMode::System:
        return SystemProxy;
    case ProxyMode::None:
        return NoProxy;
    case ProxyMode::Http:
        return HttpProxy;
    case ProxyMode::Socks5:
        return Socks5Proxy;
    }
    return SystemProxy;
}

void SettingsController::setProxyMode(int value)
{
    ProxyMode mode = ProxyMode::System;
    if (value == NoProxy)
        mode = ProxyMode::None;
    else if (value == HttpProxy)
        mode = ProxyMode::Http;
    else if (value == Socks5Proxy)
        mode = ProxyMode::Socks5;
    if (mode == m_settings.proxyMode)
        return;
    m_settings.proxyMode = mode;
    commit();
}

QString SettingsController::searchEngineId() const
{
    return QString::fromStdString(m_settings.searchEngineId);
}

QString SettingsController::customSearchTemplate() const
{
    return QString::fromStdString(m_settings.customSearchTemplate);
}

QString SettingsController::dohTemplate() const
{
    return QString::fromStdString(m_settings.dohTemplate);
}

QString SettingsController::proxyHost() const
{
    return QString::fromStdString(m_settings.proxyHost);
}

QString SettingsController::downloadDirectory() const
{
    return QString::fromStdString(m_settings.downloadDirectory);
}

#define PB_SETTER(Setter, Member, Type)                                                            \
    void SettingsController::Setter(Type value)                                                    \
    {                                                                                              \
        if (m_settings.Member == value)                                                            \
            return;                                                                                \
        m_settings.Member = value;                                                                 \
        commit();                                                                                  \
    }

PB_SETTER(setTrackerBlocking, trackerBlocking, bool)
PB_SETTER(setAdBlocking, adBlocking, bool)
PB_SETTER(setHttpsFirst, httpsFirst, bool)
PB_SETTER(setSearchSuggestions, searchSuggestions, bool)
PB_SETTER(setGlassEffects, glassEffects, bool)
PB_SETTER(setReducedMotion, reducedMotion, bool)
PB_SETTER(setAskWhereToSave, askWhereToSave, bool)
PB_SETTER(setCheckFilterListUpdatesOnStart, checkFilterListUpdatesOnStart, bool)
PB_SETTER(setTextScale, textScale, qreal)
PB_SETTER(setProxyPort, proxyPort, int)

#undef PB_SETTER

#define PB_STRING_SETTER(Setter, Member)                                                           \
    void SettingsController::Setter(const QString &value)                                          \
    {                                                                                              \
        const std::string converted = value.toStdString();                                         \
        if (m_settings.Member == converted)                                                        \
            return;                                                                                \
        m_settings.Member = converted;                                                             \
        commit();                                                                                  \
    }

PB_STRING_SETTER(setSearchEngineId, searchEngineId)
PB_STRING_SETTER(setCustomSearchTemplate, customSearchTemplate)
PB_STRING_SETTER(setDohTemplate, dohTemplate)
PB_STRING_SETTER(setProxyHost, proxyHost)
PB_STRING_SETTER(setDownloadDirectory, downloadDirectory)

#undef PB_STRING_SETTER

void SettingsController::setRememberSettings(bool value)
{
    if (m_settings.rememberSettings == value)
        return;
    m_settings.rememberSettings = value;
    if (!value)
        deleteStoredSettings();
    commit();
}

QVariantList SettingsController::searchEngines() const
{
    QVariantList list;
    for (const SearchEngine &engine : builtInSearchEngines()) {
        QVariantMap entry;
        entry[QStringLiteral("id")] = QString::fromStdString(engine.id);
        entry[QStringLiteral("name")] = QString::fromStdString(engine.name);
        entry[QStringLiteral("hasSuggestions")] = !engine.suggestionTemplate.empty();
        list.append(entry);
    }
    QVariantMap custom;
    custom[QStringLiteral("id")] = QString::fromUtf8(kCustomSearchEngineId.data(),
                                                     static_cast<int>(kCustomSearchEngineId.size()));
    custom[QStringLiteral("name")] = tr("Custom");
    custom[QStringLiteral("hasSuggestions")] = false;
    list.append(custom);
    return list;
}

QVariantList SettingsController::privacyGuarantees() const
{
    QVariantList list;
    for (const PrivacyGuarantee &guarantee : pb::settings::privacyGuarantees()) {
        QVariantMap entry;
        entry[QStringLiteral("label")]
            = QString::fromUtf8(guarantee.label.data(), static_cast<int>(guarantee.label.size()));
        entry[QStringLiteral("state")]
            = QString::fromUtf8(guarantee.state.data(), static_cast<int>(guarantee.state.size()));
        list.append(entry);
    }
    return list;
}

void SettingsController::resetToDefaults()
{
    const bool remember = m_settings.rememberSettings;
    m_settings = BrowserSettings();
    m_settings.rememberSettings = remember;
    commit();
}

} // namespace pb::settings
