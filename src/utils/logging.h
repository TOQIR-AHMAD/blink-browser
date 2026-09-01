// Privacy-safe application logging.
//
// PLAN.md section 31 forbids application logs from containing full URLs, search
// queries, cookies, POST bodies, form values, passwords, page content or
// personal information. Nothing in this project logs a URL directly; a URL that
// must appear in a diagnostic is passed through redactUrl() first.

#ifndef PB_UTILS_LOGGING_H
#define PB_UTILS_LOGGING_H

#include <string>
#include <string_view>

namespace pb::log {

enum class Level {
    Debug,
    Info,
    Warning,
    Error,
};

// How much of a URL a redacted form is allowed to keep.
enum class UrlDetail {
    // "https://<redacted>" - the only form allowed in a release build.
    SchemeOnly,
    // "https://example.com/<redacted>" - developer builds only. The host is
    // still browsing activity, so it never reaches a production log.
    SchemeAndHost,
};

// The detail level compiled into this build: SchemeAndHost only when the
// PB_DEV_LOGGING CMake option is on, SchemeOnly otherwise.
UrlDetail defaultUrlDetail() noexcept;

// Reduces a URL to a form that cannot identify what the user visited (see
// UrlDetail). Input that is not a recognisable absolute URL becomes
// "<redacted>", so a malformed value can never leak verbatim.
std::string redactUrl(std::string_view url, UrlDetail detail);
std::string redactUrl(std::string_view url);

// Writes one line to stderr. The message must already be free of user data;
// this function cannot know what a caller put in it.
void write(Level level, std::string_view message);

} // namespace pb::log

#endif // PB_UTILS_LOGGING_H
