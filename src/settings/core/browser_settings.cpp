#include "settings/core/browser_settings.h"

#include "settings/core/search_engines.h"
#include "utils/text.h"

#include <algorithm>
#include <exception>
#include <utility>

namespace pb::settings {
namespace {

using pb::privacy::CookiePolicy;
using pb::privacy::FingerprintProtection;
using pb::privacy::ProxyMode;
using pb::privacy::SecureDnsMode;

std::string boolToString(bool value)
{
    return value ? "true" : "false";
}

bool stringToBool(const std::string &value, bool fallback)
{
    if (value == "true")
        return true;
    if (value == "false")
        return false;
    return fallback;
}

template<typename T>
std::string enumToString(T value, const std::vector<std::pair<T, std::string>> &names)
{
    for (const auto &entry : names) {
        if (entry.first == value)
            return entry.second;
    }
    return names.front().second;
}

template<typename T>
T stringToEnum(const std::string &value, const std::vector<std::pair<T, std::string>> &names,
               T fallback)
{
    for (const auto &entry : names) {
        if (entry.second == value)
            return entry.first;
    }
    return fallback;
}

const std::vector<std::pair<CookiePolicy, std::string>> &cookieNames()
{
    static const std::vector<std::pair<CookiePolicy, std::string>> names = {
        { CookiePolicy::BlockThirdParty, "block-third-party" },
        { CookiePolicy::AllowAll, "allow-all" },
        { CookiePolicy::BlockAll, "block-all" },
    };
    return names;
}

const std::vector<std::pair<FingerprintProtection, std::string>> &fingerprintNames()
{
    static const std::vector<std::pair<FingerprintProtection, std::string>> names = {
        { FingerprintProtection::Standard, "standard" },
        { FingerprintProtection::Off, "off" },
        { FingerprintProtection::Strict, "strict" },
    };
    return names;
}

const std::vector<std::pair<ThemeMode, std::string>> &themeNames()
{
    static const std::vector<std::pair<ThemeMode, std::string>> names = {
        { ThemeMode::System, "system" },
        { ThemeMode::Light, "light" },
        { ThemeMode::Dark, "dark" },
    };
    return names;
}

const std::vector<std::pair<SecureDnsMode, std::string>> &dnsNames()
{
    static const std::vector<std::pair<SecureDnsMode, std::string>> names = {
        { SecureDnsMode::System, "system" },
        { SecureDnsMode::SecureDoh, "doh" },
    };
    return names;
}

const std::vector<std::pair<ProxyMode, std::string>> &proxyNames()
{
    static const std::vector<std::pair<ProxyMode, std::string>> names = {
        { ProxyMode::System, "system" },
        { ProxyMode::None, "none" },
        { ProxyMode::Http, "http" },
        { ProxyMode::Socks5, "socks5" },
    };
    return names;
}

std::string lookup(const std::map<std::string, std::string> &values, const std::string &key)
{
    const auto it = values.find(key);
    return it == values.end() ? std::string() : it->second;
}

} // namespace

void BrowserSettings::sanitize()
{
    textScale = std::min(2.0, std::max(0.8, textScale));

    if (!customSearchTemplate.empty() && !isValidSearchTemplate(customSearchTemplate))
        customSearchTemplate.clear();
    if (searchEngineId == std::string(kCustomSearchEngineId)) {
        if (customSearchTemplate.empty())
            searchEngineId = std::string(defaultSearchEngineId());
    } else if (!findSearchEngine(searchEngineId)) {
        searchEngineId = std::string(defaultSearchEngineId());
    }

    if (dnsMode == SecureDnsMode::SecureDoh
        && !pb::text::startsWith(pb::text::toLower(dohTemplate), "https://")) {
        // Refusing here rather than silently sending DNS in the clear.
        dnsMode = SecureDnsMode::System;
        dohTemplate.clear();
    }

    if (proxyPort < 0 || proxyPort > 65535)
        proxyPort = 0;
    if ((proxyMode == ProxyMode::Http || proxyMode == ProxyMode::Socks5)
        && (proxyHost.empty() || proxyPort == 0)) {
        proxyMode = ProxyMode::System;
    }
}

std::map<std::string, std::string> BrowserSettings::toMap() const
{
    std::map<std::string, std::string> values;
    values["privacy.trackerBlocking"] = boolToString(trackerBlocking);
    values["privacy.adBlocking"] = boolToString(adBlocking);
    values["privacy.cookiePolicy"] = enumToString(cookiePolicy, cookieNames());
    values["privacy.fingerprintProtection"]
        = enumToString(fingerprintProtection, fingerprintNames());
    values["privacy.httpsFirst"] = boolToString(httpsFirst);
    values["privacy.searchSuggestions"] = boolToString(searchSuggestions);
    values["privacy.rememberSettings"] = boolToString(rememberSettings);

    values["appearance.theme"] = enumToString(theme, themeNames());
    values["appearance.glassEffects"] = boolToString(glassEffects);
    values["appearance.reducedMotion"] = boolToString(reducedMotion);
    values["appearance.textScale"] = std::to_string(textScale);

    values["search.engineId"] = searchEngineId;
    values["search.customTemplate"] = customSearchTemplate;

    values["network.dnsMode"] = enumToString(dnsMode, dnsNames());
    values["network.dohTemplate"] = dohTemplate;
    values["network.proxyMode"] = enumToString(proxyMode, proxyNames());
    values["network.proxyHost"] = proxyHost;
    values["network.proxyPort"] = std::to_string(proxyPort);

    values["downloads.askWhereToSave"] = boolToString(askWhereToSave);
    values["downloads.directory"] = downloadDirectory;

    values["blocking.checkUpdatesOnStart"] = boolToString(checkFilterListUpdatesOnStart);
    return values;
}

BrowserSettings BrowserSettings::fromMap(const std::map<std::string, std::string> &values)
{
    BrowserSettings settings;
    const BrowserSettings defaults;

    settings.trackerBlocking
        = stringToBool(lookup(values, "privacy.trackerBlocking"), defaults.trackerBlocking);
    settings.adBlocking = stringToBool(lookup(values, "privacy.adBlocking"), defaults.adBlocking);
    settings.cookiePolicy = stringToEnum(lookup(values, "privacy.cookiePolicy"), cookieNames(),
                                         defaults.cookiePolicy);
    settings.fingerprintProtection
        = stringToEnum(lookup(values, "privacy.fingerprintProtection"), fingerprintNames(),
                       defaults.fingerprintProtection);
    settings.httpsFirst = stringToBool(lookup(values, "privacy.httpsFirst"), defaults.httpsFirst);
    settings.searchSuggestions
        = stringToBool(lookup(values, "privacy.searchSuggestions"), defaults.searchSuggestions);
    settings.rememberSettings
        = stringToBool(lookup(values, "privacy.rememberSettings"), defaults.rememberSettings);

    settings.theme = stringToEnum(lookup(values, "appearance.theme"), themeNames(),
                                  defaults.theme);
    settings.glassEffects
        = stringToBool(lookup(values, "appearance.glassEffects"), defaults.glassEffects);
    settings.reducedMotion
        = stringToBool(lookup(values, "appearance.reducedMotion"), defaults.reducedMotion);
    const std::string scale = lookup(values, "appearance.textScale");
    if (!scale.empty()) {
        try {
            settings.textScale = std::stod(scale);
        } catch (const std::exception &) {
            settings.textScale = defaults.textScale;
        }
    }

    const std::string engineId = lookup(values, "search.engineId");
    if (!engineId.empty())
        settings.searchEngineId = engineId;
    settings.customSearchTemplate = lookup(values, "search.customTemplate");

    settings.dnsMode = stringToEnum(lookup(values, "network.dnsMode"), dnsNames(),
                                    defaults.dnsMode);
    settings.dohTemplate = lookup(values, "network.dohTemplate");
    settings.proxyMode = stringToEnum(lookup(values, "network.proxyMode"), proxyNames(),
                                      defaults.proxyMode);
    settings.proxyHost = lookup(values, "network.proxyHost");
    const std::string port = lookup(values, "network.proxyPort");
    if (!port.empty()) {
        try {
            settings.proxyPort = std::stoi(port);
        } catch (const std::exception &) {
            settings.proxyPort = 0;
        }
    }

    settings.askWhereToSave
        = stringToBool(lookup(values, "downloads.askWhereToSave"), defaults.askWhereToSave);
    settings.downloadDirectory = lookup(values, "downloads.directory");

    settings.checkFilterListUpdatesOnStart
        = stringToBool(lookup(values, "blocking.checkUpdatesOnStart"),
                       defaults.checkFilterListUpdatesOnStart);

    settings.sanitize();
    return settings;
}

std::string BrowserSettings::searchTemplate() const
{
    if (searchEngineId == std::string(kCustomSearchEngineId)
        && isValidSearchTemplate(customSearchTemplate)) {
        return customSearchTemplate;
    }
    if (const SearchEngine *engine = findSearchEngine(searchEngineId))
        return engine->queryTemplate;
    const SearchEngine *fallback = findSearchEngine(defaultSearchEngineId());
    return fallback ? fallback->queryTemplate : std::string();
}

std::string BrowserSettings::suggestionTemplate() const
{
    if (!searchSuggestions)
        return {};
    if (searchEngineId == std::string(kCustomSearchEngineId))
        return {}; // no way to know a custom provider's suggestion endpoint
    if (const SearchEngine *engine = findSearchEngine(searchEngineId))
        return engine->suggestionTemplate;
    return {};
}

const std::vector<PrivacyGuarantee> &privacyGuarantees()
{
    static const std::vector<PrivacyGuarantee> guarantees = {
        { "Telemetry", "Not implemented" },
        { "Analytics", "Not implemented" },
        { "Crash report upload", "Not implemented" },
        { "Cloud sync", "Not implemented" },
        { "User account", "Not implemented" },
        { "Advertising ID", "Not implemented" },
        { "Installation ID", "Not implemented" },
        { "History persistence", "Off - session only" },
        { "Cookie persistence", "Off - session only" },
        { "Cache persistence", "Off - memory only" },
        { "Password storage", "Not implemented" },
    };
    return guarantees;
}

} // namespace pb::settings
