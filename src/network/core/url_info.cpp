#include "network/core/url_info.h"

#include "utils/text.h"

#include <algorithm>
#include <array>

namespace pb::net {
namespace {

// Common two-label public suffixes. Not the full Public Suffix List; see the
// comment on UrlInfo::registrableDomain().
constexpr std::array<std::string_view, 40> kTwoLabelSuffixes = {
    "co.uk",  "org.uk", "ac.uk",  "gov.uk", "me.uk",  "net.uk", "sch.uk", "com.au",
    "net.au", "org.au", "edu.au", "gov.au", "co.nz",  "net.nz", "org.nz", "co.jp",
    "ne.jp",  "or.jp",  "ac.jp",  "go.jp",  "co.kr",  "or.kr",  "com.br", "net.br",
    "org.br", "gov.br", "com.cn", "net.cn", "org.cn", "gov.cn", "com.mx", "co.in",
    "net.in", "org.in", "co.za",  "com.tr", "com.sg", "com.hk", "com.tw", "co.il",
};

bool isDigits(std::string_view value)
{
    return !value.empty()
        && std::all_of(value.begin(), value.end(), [](char c) { return c >= '0' && c <= '9'; });
}

bool looksLikeIpAddress(std::string_view host)
{
    if (host.find(':') != std::string_view::npos)
        return true; // IPv6 literal
    const auto labels = pb::text::split(host, '.');
    if (labels.size() != 4)
        return false;
    return std::all_of(labels.begin(), labels.end(), isDigits);
}

} // namespace

UrlInfo UrlInfo::parse(std::string_view url)
{
    UrlInfo info;

    const std::size_t colon = url.find(':');
    if (colon == std::string_view::npos || colon == 0)
        return info;

    for (std::size_t i = 0; i < colon; ++i) {
        const char c = url[i];
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
            || c == '+' || c == '-' || c == '.';
        if (!ok)
            return info;
    }

    info.valid = true;
    info.scheme = pb::text::toLower(url.substr(0, colon));

    std::string_view rest = url.substr(colon + 1);
    if (!pb::text::startsWith(rest, "//")) {
        // Opaque scheme (about:, data:, mailto:, ...): everything after the
        // scheme is scheme-specific and is kept in `path` untouched.
        info.path = std::string(rest);
        return info;
    }
    rest.remove_prefix(2);

    const std::size_t fragmentAt = rest.find('#');
    if (fragmentAt != std::string_view::npos) {
        info.fragment = std::string(rest.substr(fragmentAt + 1));
        rest = rest.substr(0, fragmentAt);
    }

    const std::size_t queryAt = rest.find('?');
    if (queryAt != std::string_view::npos) {
        info.query = std::string(rest.substr(queryAt + 1));
        rest = rest.substr(0, queryAt);
    }

    std::string_view authority = rest;
    const std::size_t pathAt = rest.find('/');
    if (pathAt != std::string_view::npos) {
        authority = rest.substr(0, pathAt);
        info.path = std::string(rest.substr(pathAt));
    }

    const std::size_t credentialsAt = authority.rfind('@');
    if (credentialsAt != std::string_view::npos)
        authority = authority.substr(credentialsAt + 1);

    if (!authority.empty() && authority.front() == '[') {
        // IPv6 literal: the port separator is the colon after "]".
        const std::size_t close = authority.find(']');
        if (close != std::string_view::npos) {
            info.host = pb::text::toLower(authority.substr(1, close - 1));
            const std::string_view tail = authority.substr(close + 1);
            if (pb::text::startsWith(tail, ":"))
                info.port = std::string(tail.substr(1));
            return info;
        }
    }

    const std::size_t portAt = authority.rfind(':');
    if (portAt != std::string_view::npos) {
        info.host = pb::text::toLower(authority.substr(0, portAt));
        info.port = std::string(authority.substr(portAt + 1));
    } else {
        info.host = pb::text::toLower(authority);
    }

    // A trailing dot ("example.com.") is the same site as "example.com".
    if (!info.host.empty() && info.host.back() == '.')
        info.host.pop_back();

    return info;
}

bool UrlInfo::isSecureScheme() const
{
    return scheme == "https" || scheme == "wss";
}

bool UrlInfo::isHttpFamily() const
{
    return scheme == "http" || scheme == "https" || scheme == "ws" || scheme == "wss";
}

std::string UrlInfo::hostWithPort() const
{
    return port.empty() ? host : host + ":" + port;
}

std::string UrlInfo::registrableDomain() const
{
    return registrableDomainOf(host);
}

std::string registrableDomainOf(std::string_view host)
{
    if (host.empty())
        return {};
    if (looksLikeIpAddress(host))
        return std::string(host);

    const auto labels = pb::text::split(host, '.');
    if (labels.size() <= 2)
        return std::string(host);

    std::size_t suffixLabels = 1;
    const std::string lastTwo = std::string(labels[labels.size() - 2]) + "."
        + std::string(labels[labels.size() - 1]);
    if (std::find(kTwoLabelSuffixes.begin(), kTwoLabelSuffixes.end(), lastTwo)
        != kTwoLabelSuffixes.end()) {
        suffixLabels = 2;
    }

    if (labels.size() <= suffixLabels + 1)
        return std::string(host);

    std::string domain;
    for (std::size_t i = labels.size() - suffixLabels - 1; i < labels.size(); ++i) {
        if (!domain.empty())
            domain.push_back('.');
        domain.append(labels[i]);
    }
    return domain;
}

bool isThirdParty(const UrlInfo &request, std::string_view firstPartyHost)
{
    if (firstPartyHost.empty() || request.host.empty())
        return true;
    const std::string requestDomain = request.registrableDomain();
    const std::string documentDomain = registrableDomainOf(pb::text::toLower(firstPartyHost));
    if (requestDomain.empty() || documentDomain.empty())
        return true;
    return requestDomain != documentDomain;
}

} // namespace pb::net
