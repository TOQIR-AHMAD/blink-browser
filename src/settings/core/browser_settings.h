// Every user-visible setting, in one Qt-free structure.
//
// Two rules shape this file:
//
// 1. The defaults are the privacy-preserving values. A fresh profile - which
//    is every launch unless the user opts into remembering settings - is the
//    safe configuration (PLAN.md §32).
// 2. There is no telemetry, crash-upload, cloud-sync or history-persistence
//    setting, not even one defaulted to off. A setting that does not exist
//    cannot be flipped by a bad default, a migration or a bug (PLAN.md §29).
//    The privacy page reports those as permanently off by reading the
//    constants at the bottom of this header.

#ifndef PB_SETTINGS_CORE_BROWSER_SETTINGS_H
#define PB_SETTINGS_CORE_BROWSER_SETTINGS_H

#include "privacy/core/privacy_types.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace pb::settings {

enum class ThemeMode {
    System,
    Light,
    Dark,
};

struct BrowserSettings {
    // --- Privacy -----------------------------------------------------------
    bool trackerBlocking = true;
    bool adBlocking = true;
    pb::privacy::CookiePolicy cookiePolicy = pb::privacy::CookiePolicy::BlockThirdParty;
    pb::privacy::FingerprintProtection fingerprintProtection
        = pb::privacy::FingerprintProtection::Standard;
    bool httpsFirst = true;

    // Off by default: every keystroke would otherwise go to the search
    // provider (PLAN.md §10, §50).
    bool searchSuggestions = false;

    // Off by default: when on, the settings below - and only the settings,
    // never browsing data - are written to a JSON file in the user's config
    // directory (PLAN.md §57 requires this to be a deliberate choice).
    bool rememberSettings = false;

    // --- Appearance --------------------------------------------------------
    ThemeMode theme = ThemeMode::System;
    bool glassEffects = true;
    bool reducedMotion = false;
    double textScale = 1.0;

    // --- Search ------------------------------------------------------------
    std::string searchEngineId = "duckduckgo";
    std::string customSearchTemplate;

    // --- Network -----------------------------------------------------------
    pb::privacy::SecureDnsMode dnsMode = pb::privacy::SecureDnsMode::System;
    std::string dohTemplate;
    pb::privacy::ProxyMode proxyMode = pb::privacy::ProxyMode::System;
    std::string proxyHost;
    int proxyPort = 0;

    // --- Downloads ---------------------------------------------------------
    bool askWhereToSave = true;
    std::string downloadDirectory;

    // --- Blocking ----------------------------------------------------------
    // Manual by default: a filter-list update is a network request to a third
    // party, and the user decides when to make it (PLAN.md §21).
    bool checkFilterListUpdatesOnStart = false;
    std::vector<std::string> extraFilterListPaths;

    // Clamps out-of-range values and drops settings that would be unsafe
    // (a non-https search or DoH template, a nonsensical text scale).
    void sanitize();

    std::map<std::string, std::string> toMap() const;
    static BrowserSettings fromMap(const std::map<std::string, std::string> &values);

    // The effective search template: the custom one when it is valid and
    // selected, otherwise the built-in provider's.
    std::string searchTemplate() const;
    std::string suggestionTemplate() const;
};

// Things the browser does not do, exposed so the privacy page states them from
// one source instead of hard-coding strings in QML.
struct PrivacyGuarantee {
    std::string_view label;
    std::string_view state; // always the same: these are not settings
};
const std::vector<PrivacyGuarantee> &privacyGuarantees();

constexpr std::string_view kCustomSearchEngineId = "custom";

} // namespace pb::settings

#endif // PB_SETTINGS_CORE_BROWSER_SETTINGS_H
