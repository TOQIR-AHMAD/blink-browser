#include "logging.h"

#include <cstdio>

namespace pb::log {
namespace {

constexpr std::string_view kRedacted = "<redacted>";

bool isSchemeChar(char c) noexcept
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
        || c == '+' || c == '-' || c == '.';
}

// Returns the length of a valid scheme at the start of `url`, or 0.
std::size_t schemeLength(std::string_view url) noexcept
{
    const std::size_t colon = url.find(':');
    if (colon == std::string_view::npos || colon == 0)
        return 0;
    for (std::size_t i = 0; i < colon; ++i) {
        if (!isSchemeChar(url[i]))
            return 0;
    }
    return colon;
}

const char *levelName(Level level) noexcept
{
    switch (level) {
    case Level::Debug:
        return "debug";
    case Level::Info:
        return "info";
    case Level::Warning:
        return "warning";
    case Level::Error:
        return "error";
    }
    return "info";
}

} // namespace

UrlDetail defaultUrlDetail() noexcept
{
#ifdef PB_DEV_LOGGING
    return UrlDetail::SchemeAndHost;
#else
    return UrlDetail::SchemeOnly;
#endif
}

std::string redactUrl(std::string_view url, UrlDetail detail)
{
    const std::size_t scheme = schemeLength(url);
    if (scheme == 0)
        return std::string(kRedacted);

    const std::string_view schemeText = url.substr(0, scheme);
    std::string_view rest = url.substr(scheme + 1);

    // Opaque schemes (about:, data:, javascript:, ...) carry no authority and
    // their remainder can be arbitrary content, so nothing after the scheme is
    // ever kept.
    if (rest.substr(0, 2) != "//")
        return std::string(schemeText) + ":" + std::string(kRedacted);
    rest.remove_prefix(2);

    if (detail == UrlDetail::SchemeOnly)
        return std::string(schemeText) + "://" + std::string(kRedacted);

    const std::size_t authorityEnd = rest.find_first_of("/?#");
    std::string_view authority = rest.substr(0, authorityEnd);
    const bool hasMore = authorityEnd != std::string_view::npos && authorityEnd + 1 < rest.size();

    // Credentials are never logged, at any detail level.
    const std::size_t at = authority.rfind('@');
    if (at != std::string_view::npos)
        authority = authority.substr(at + 1);

    if (authority.empty())
        return std::string(schemeText) + "://" + std::string(kRedacted);

    std::string out = std::string(schemeText) + "://" + std::string(authority);
    if (hasMore) {
        out += '/';
        out += kRedacted;
    }
    return out;
}

std::string redactUrl(std::string_view url)
{
    return redactUrl(url, defaultUrlDetail());
}

void write(Level level, std::string_view message)
{
    std::fprintf(stderr, "[%s] %.*s\n", levelName(level), static_cast<int>(message.size()),
                 message.data());
}

} // namespace pb::log
