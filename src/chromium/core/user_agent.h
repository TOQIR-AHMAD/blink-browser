// User-agent reduction.
//
// Qt WebEngine's default user agent contains a "QtWebEngine/6.x.y" token, which
// is rare enough on the open web to be a strong fingerprint on its own. The
// browser therefore reports the plain Chrome user agent for the platform, with
// the real Chromium major version taken from Qt's own string so the value stays
// truthful (PLAN.md §22: reduce entropy, do not lie about capabilities).

#ifndef PB_CHROMIUM_CORE_USER_AGENT_H
#define PB_CHROMIUM_CORE_USER_AGENT_H

#include <string>
#include <string_view>

namespace pb::chromium {

enum class UserAgentPlatform {
    Windows,
    MacOs,
    Linux,
};

// Extracts the Chromium major version ("130") from a user agent containing
// "Chrome/130.0.6723.59". Returns an empty string when absent.
std::string chromiumMajorVersion(std::string_view defaultUserAgent);

// Returns the reduced user agent, or an empty string when the Chromium version
// could not be determined - in that case the caller keeps Qt's default rather
// than inventing a version number.
std::string reducedUserAgent(std::string_view defaultUserAgent, UserAgentPlatform platform);

} // namespace pb::chromium

#endif // PB_CHROMIUM_CORE_USER_AGENT_H
