// Shared privacy vocabulary.
//
// These enums are referenced by the Chromium layer, the settings model and the
// UI, so they live in one Qt-free header to keep a single definition of what
// each policy means.

#ifndef PB_PRIVACY_CORE_PRIVACY_TYPES_H
#define PB_PRIVACY_CORE_PRIVACY_TYPES_H

#include <string_view>

namespace pb::privacy {

enum class CookiePolicy {
    AllowAll,        // first- and third-party cookies, session lifetime only
    BlockThirdParty, // default
    BlockAll,
};

// See docs/privacy.md for exactly what each level changes and what it can
// break. PLAN.md §22 puts compatibility ahead of aggressive breakage, so
// Standard is the default and Strict is opt-in.
enum class FingerprintProtection {
    Off,
    Standard, // user-agent reduction, no local WebRTC IP exposure
    Strict,   // adds canvas/audio read protection and a fixed accept-language
};

enum class SecureDnsMode {
    System,    // whatever the operating system is configured to use
    SecureDoh, // the DNS-over-HTTPS template the user chose
};

enum class ProxyMode {
    System,
    None,
    Http,
    Socks5,
};

// Cookie lifetime is never configurable: nothing this browser stores survives
// the session, so there is no "keep cookies" option to get wrong.
constexpr std::string_view kCookieLifetimeDescription = "session only";

} // namespace pb::privacy

#endif // PB_PRIVACY_CORE_PRIVACY_TYPES_H
