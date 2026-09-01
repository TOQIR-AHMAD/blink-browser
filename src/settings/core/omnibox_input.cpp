#include "settings/core/omnibox_input.h"

#include "network/core/url_info.h"
#include "utils/text.h"

#include <algorithm>

namespace pb::settings {
namespace {

bool isKnownScheme(std::string_view scheme)
{
    return scheme == "http" || scheme == "https" || scheme == "file" || scheme == "about"
        || scheme == "ftp" || scheme == "ws" || scheme == "wss" || scheme == "view-source";
}

bool hasWhitespace(std::string_view value)
{
    return std::any_of(value.begin(), value.end(),
                       [](char c) { return c == ' ' || c == '\t'; });
}

bool looksLikeWindowsPath(std::string_view value)
{
    return value.size() >= 3 && ((value[0] >= 'a' && value[0] <= 'z')
                                 || (value[0] >= 'A' && value[0] <= 'Z'))
        && value[1] == ':' && (value[2] == '\\' || value[2] == '/');
}

bool isIpv4(std::string_view host)
{
    const auto labels = pb::text::split(host, '.');
    if (labels.size() != 4)
        return false;
    return std::all_of(labels.begin(), labels.end(), [](std::string_view label) {
        return !label.empty() && label.size() <= 3
            && std::all_of(label.begin(), label.end(),
                           [](char c) { return c >= '0' && c <= '9'; });
    });
}

// Splits a "host[:port]" candidate, or returns an empty view when the port is
// not numeric.
std::string_view hostPart(std::string_view candidate, bool &ok)
{
    ok = true;
    const std::size_t colon = candidate.rfind(':');
    if (colon == std::string_view::npos)
        return candidate;
    const std::string_view port = candidate.substr(colon + 1);
    const bool numericPort = !port.empty()
        && std::all_of(port.begin(), port.end(), [](char c) { return c >= '0' && c <= '9'; });
    if (!numericPort) {
        ok = false;
        return {};
    }
    return candidate.substr(0, colon);
}

// Local addresses are reached over plain http: an https-first guess just adds
// a failed connection on every "localhost:3000".
bool prefersHttp(std::string_view candidate)
{
    bool ok = false;
    const std::string_view host = hostPart(candidate, ok);
    if (!ok)
        return false;
    return host == "localhost" || isIpv4(host);
}

bool isHostLike(std::string_view candidate)
{
    bool ok = false;
    const std::string_view host = hostPart(candidate, ok);
    if (!ok)
        return false;

    if (host == "localhost")
        return true;
    if (host.empty() || host.front() == '.' || host.back() == '.')
        return false;
    if (isIpv4(host))
        return true;

    const auto labels = pb::text::split(host, '.');
    if (labels.size() < 2)
        return false;

    for (const std::string_view label : labels) {
        if (label.empty())
            return false;
        for (const char c : label) {
            const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '-' || c == '_';
            if (!ok)
                return false;
        }
    }

    // The last label has to look like a TLD: letters only, at least two of
    // them. This is what keeps "version 1.2" a search and "example.io" a URL.
    const std::string_view tld = labels.back();
    if (tld.size() < 2)
        return false;
    return std::all_of(tld.begin(), tld.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    });
}

} // namespace

bool isDangerousScheme(std::string_view scheme)
{
    const std::string lowered = pb::text::toLower(scheme);
    return lowered == "javascript" || lowered == "data" || lowered == "blob"
        || lowered == "filesystem";
}

OmniboxResult classifyInput(std::string_view input)
{
    const std::string_view trimmed = pb::text::trim(input);
    OmniboxResult result;
    if (trimmed.empty())
        return result;

    const pb::net::UrlInfo parsed = pb::net::UrlInfo::parse(trimmed);
    if (parsed.valid && !parsed.scheme.empty()) {
        if (isDangerousScheme(parsed.scheme)) {
            // Pasting "javascript:..." into the address bar is a classic
            // self-XSS delivery route. It becomes a search instead.
            result.action = OmniboxAction::Search;
            result.value = std::string(trimmed);
            return result;
        }
        if (isKnownScheme(parsed.scheme)) {
            result.action = OmniboxAction::Navigate;
            result.value = std::string(trimmed);
            return result;
        }
        // An unknown scheme with an authority ("myapp://host") is still a
        // navigation attempt; a bare "word:word" is not.
        if (!parsed.host.empty()) {
            result.action = OmniboxAction::Navigate;
            result.value = std::string(trimmed);
            return result;
        }
    }

    if (looksLikeWindowsPath(trimmed) || pb::text::startsWith(trimmed, "/")) {
        std::string path(trimmed);
        std::replace(path.begin(), path.end(), '\\', '/');
        result.action = OmniboxAction::Navigate;
        result.value = pb::text::startsWith(path, "/") ? "file://" + path : "file:///" + path;
        return result;
    }

    if (!hasWhitespace(trimmed)) {
        // Only the authority part is examined: "example.com/a b" never reaches
        // here because of the space, and "example.com/path" splits cleanly.
        const std::size_t slash = trimmed.find('/');
        const std::string_view authority
            = slash == std::string_view::npos ? trimmed : trimmed.substr(0, slash);
        if (isHostLike(authority)) {
            result.action = OmniboxAction::Navigate;
            // https first; the HTTPS-first logic downgrades only if the host
            // genuinely has no https endpoint.
            const std::string_view scheme = prefersHttp(authority) ? "http://" : "https://";
            result.value = std::string(scheme) + std::string(trimmed);
            return result;
        }
    }

    result.action = OmniboxAction::Search;
    result.value = std::string(trimmed);
    return result;
}

} // namespace pb::settings
