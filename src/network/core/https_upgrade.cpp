#include "network/core/https_upgrade.h"

#include "utils/text.h"

#include <algorithm>

namespace pb::net {
namespace {

bool startsWithNumber(std::string_view host, int first, int secondLow, int secondHigh)
{
    const auto labels = pb::text::split(host, '.');
    if (labels.size() != 4)
        return false;
    const auto value = [](std::string_view label) {
        int number = 0;
        for (const char c : label) {
            if (c < '0' || c > '9')
                return -1;
            number = number * 10 + (c - '0');
        }
        return number;
    };
    const int a = value(labels[0]);
    const int b = value(labels[1]);
    if (a != first)
        return false;
    return b >= secondLow && b <= secondHigh;
}

} // namespace

bool isLocalHost(std::string_view host)
{
    if (host.empty())
        return false;
    if (host == "localhost" || pb::text::endsWith(host, ".localhost"))
        return true;
    if (host == "127.0.0.1" || host == "::1")
        return true;
    if (pb::text::endsWith(host, ".local") || pb::text::endsWith(host, ".internal"))
        return true;
    // RFC 1918 ranges.
    if (startsWithNumber(host, 10, 0, 255))
        return true;
    if (startsWithNumber(host, 192, 168, 168))
        return true;
    if (startsWithNumber(host, 172, 16, 31))
        return true;
    if (startsWithNumber(host, 127, 0, 255))
        return true;
    // A single-label host is an intranet name, not a public site.
    return host.find('.') == std::string_view::npos;
}

std::string HttpsUpgrade::upgradedUrl(const UrlInfo &url, bool isMainFrame) const
{
    if (!isMainFrame || !url.valid || url.scheme != "http" || url.host.empty())
        return {};
    if (isLocalHost(url.host) || isKnownHttpOnly(url.host))
        return {};

    std::string upgraded = "https://" + url.host;
    if (!url.port.empty() && url.port != "80")
        upgraded += ":" + url.port;
    upgraded += url.path.empty() ? "/" : url.path;
    if (!url.query.empty())
        upgraded += "?" + url.query;
    if (!url.fragment.empty())
        upgraded += "#" + url.fragment;
    return upgraded;
}

void HttpsUpgrade::recordFailure(std::string_view host)
{
    if (host.empty())
        return;
    m_httpOnlyHosts.insert(pb::text::toLower(host));
}

bool HttpsUpgrade::isKnownHttpOnly(std::string_view host) const
{
    return m_httpOnlyHosts.find(pb::text::toLower(host)) != m_httpOnlyHosts.end();
}

void HttpsUpgrade::clear()
{
    m_httpOnlyHosts.clear();
}

} // namespace pb::net
