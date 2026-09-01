#include "check.h"
#include "settings/core/browser_settings.h"

using namespace pb::settings;
using pb::privacy::CookiePolicy;
using pb::privacy::FingerprintProtection;
using pb::privacy::ProxyMode;
using pb::privacy::SecureDnsMode;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

void defaultsAreThePrivateOnes()
{
    const BrowserSettings settings;
    checkTrue(settings.trackerBlocking, "tracker blocking on by default");
    checkTrue(settings.adBlocking, "ad blocking on by default");
    checkTrue(settings.cookiePolicy == CookiePolicy::BlockThirdParty,
              "third-party cookies blocked by default");
    checkTrue(settings.fingerprintProtection == FingerprintProtection::Standard,
              "fingerprint protection on, but not the compatibility-breaking level");
    checkTrue(settings.httpsFirst, "https first by default");
    checkTrue(!settings.searchSuggestions, "search suggestions are opt-in");
    checkTrue(!settings.rememberSettings, "settings persistence is opt-in");
    checkTrue(!settings.checkFilterListUpdatesOnStart, "filter updates are manual by default");
    checkTrue(settings.askWhereToSave, "downloads ask where to save");
    checkEqual(settings.searchEngineId, "duckduckgo", "default provider");
}

void roundTrips()
{
    BrowserSettings settings;
    settings.trackerBlocking = false;
    settings.cookiePolicy = CookiePolicy::BlockAll;
    settings.fingerprintProtection = FingerprintProtection::Strict;
    settings.theme = ThemeMode::Dark;
    settings.textScale = 1.25;
    settings.searchEngineId = "startpage";
    settings.dnsMode = SecureDnsMode::SecureDoh;
    settings.dohTemplate = "https://dns.example/dns-query";
    settings.proxyMode = ProxyMode::Socks5;
    settings.proxyHost = "127.0.0.1";
    settings.proxyPort = 9050;
    settings.rememberSettings = true;

    const BrowserSettings restored = BrowserSettings::fromMap(settings.toMap());
    checkTrue(restored.trackerBlocking == false, "bool round trip");
    checkTrue(restored.cookiePolicy == CookiePolicy::BlockAll, "cookie policy round trip");
    checkTrue(restored.fingerprintProtection == FingerprintProtection::Strict,
              "fingerprint round trip");
    checkTrue(restored.theme == ThemeMode::Dark, "theme round trip");
    checkTrue(restored.textScale > 1.24 && restored.textScale < 1.26, "text scale round trip");
    checkEqual(restored.searchEngineId, "startpage", "engine round trip");
    checkTrue(restored.dnsMode == SecureDnsMode::SecureDoh, "dns round trip");
    checkEqual(restored.dohTemplate, "https://dns.example/dns-query", "doh template round trip");
    checkTrue(restored.proxyMode == ProxyMode::Socks5, "proxy round trip");
    checkEqual(static_cast<long long>(restored.proxyPort), 9050, "proxy port round trip");
    checkTrue(restored.rememberSettings, "remember flag round trip");
}

void unknownAndHostileValuesFallBack()
{
    std::map<std::string, std::string> values;
    values["privacy.cookiePolicy"] = "allow-everything-always";
    values["appearance.theme"] = "neon";
    values["appearance.textScale"] = "not-a-number";
    values["search.engineId"] = "evil-search";
    values["network.proxyPort"] = "999999";

    const BrowserSettings settings = BrowserSettings::fromMap(values);
    checkTrue(settings.cookiePolicy == CookiePolicy::BlockThirdParty,
              "an unknown cookie policy falls back to the safe default");
    checkTrue(settings.theme == ThemeMode::System, "unknown theme falls back");
    checkTrue(settings.textScale == 1.0, "unparsable scale falls back");
    checkEqual(settings.searchEngineId, "duckduckgo", "unknown search engine falls back");
    checkEqual(static_cast<long long>(settings.proxyPort), 0, "out-of-range port dropped");
}

void sanitizeRefusesUnsafeConfigurations()
{
    BrowserSettings insecureDoh;
    insecureDoh.dnsMode = SecureDnsMode::SecureDoh;
    insecureDoh.dohTemplate = "http://dns.example/dns-query";
    insecureDoh.sanitize();
    checkTrue(insecureDoh.dnsMode == SecureDnsMode::System,
              "a plain-http DoH template disables secure DNS instead of leaking queries");

    BrowserSettings halfProxy;
    halfProxy.proxyMode = ProxyMode::Http;
    halfProxy.proxyHost = "proxy.example";
    halfProxy.proxyPort = 0;
    halfProxy.sanitize();
    checkTrue(halfProxy.proxyMode == ProxyMode::System, "an incomplete proxy is not applied");

    BrowserSettings badCustom;
    badCustom.searchEngineId = "custom";
    badCustom.customSearchTemplate = "http://searchy.example/?q={searchTerms}";
    badCustom.sanitize();
    checkEqual(badCustom.searchEngineId, "duckduckgo",
               "an http custom search template is refused");

    BrowserSettings hugeScale;
    hugeScale.textScale = 99.0;
    hugeScale.sanitize();
    checkTrue(hugeScale.textScale == 2.0, "text scale clamped");
}

void searchTemplateSelection()
{
    BrowserSettings settings;
    checkTrue(settings.searchTemplate().find("duckduckgo") != std::string::npos,
              "default template");
    checkEqual(settings.suggestionTemplate(), "",
               "no suggestion endpoint while suggestions are off");

    settings.searchSuggestions = true;
    checkTrue(!settings.suggestionTemplate().empty(),
              "suggestion endpoint available once opted in");

    settings.searchEngineId = "custom";
    settings.customSearchTemplate = "https://my.example/?q={searchTerms}";
    checkEqual(settings.searchTemplate(), "https://my.example/?q={searchTerms}",
               "custom template used");
    checkEqual(settings.suggestionTemplate(), "",
               "a custom provider gets no suggestion endpoint");
}

void thereAreNoSettingsForThingsThatDoNotExist()
{
    // toMap() is the full persisted surface: if a telemetry or sync key ever
    // appears here, this test is the thing that fails.
    const auto values = BrowserSettings().toMap();
    for (const auto &entry : values) {
        const std::string &key = entry.first;
        checkTrue(key.find("telemetry") == std::string::npos, "no telemetry key");
        checkTrue(key.find("analytics") == std::string::npos, "no analytics key");
        checkTrue(key.find("sync") == std::string::npos, "no sync key");
        checkTrue(key.find("account") == std::string::npos, "no account key");
        checkTrue(key.find("crashUpload") == std::string::npos, "no crash upload key");
        checkTrue(key.find("history") == std::string::npos, "no history persistence key");
    }
    checkTrue(privacyGuarantees().size() >= 10, "the privacy page has something to state");
}

} // namespace

int main()
{
    defaultsAreThePrivateOnes();
    roundTrips();
    unknownAndHostileValuesFallBack();
    sanitizeRefusesUnsafeConfigurations();
    searchTemplateSelection();
    thereAreNoSettingsForThingsThatDoNotExist();
    return pbtest::finish();
}
