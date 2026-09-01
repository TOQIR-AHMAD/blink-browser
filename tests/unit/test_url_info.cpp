#include "check.h"
#include "network/core/url_info.h"

using pb::net::isThirdParty;
using pb::net::registrableDomainOf;
using pb::net::UrlInfo;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

void parsing()
{
    const UrlInfo full = UrlInfo::parse("HTTPS://User:pw@Example.COM:8443/a/b?q=1#frag");
    checkTrue(full.valid, "full URL parses");
    checkEqual(full.scheme, "https", "scheme lower-cased");
    checkEqual(full.host, "example.com", "host lower-cased, credentials removed");
    checkEqual(full.port, "8443", "port");
    checkEqual(full.path, "/a/b", "path");
    checkEqual(full.query, "q=1", "query");
    checkEqual(full.fragment, "frag", "fragment");
    checkEqual(full.hostWithPort(), "example.com:8443", "hostWithPort");
    checkTrue(full.isSecureScheme(), "https is secure");
    checkTrue(full.isHttpFamily(), "https is http family");

    const UrlInfo bare = UrlInfo::parse("http://example.com");
    checkEqual(bare.host, "example.com", "bare host");
    checkEqual(bare.path, "", "no path");
    checkTrue(!bare.isSecureScheme(), "http is not secure");

    const UrlInfo trailingDot = UrlInfo::parse("https://example.com./x");
    checkEqual(trailingDot.host, "example.com", "trailing dot dropped");

    const UrlInfo ipv6 = UrlInfo::parse("http://[::1]:8080/x");
    checkEqual(ipv6.host, "::1", "ipv6 literal");
    checkEqual(ipv6.port, "8080", "ipv6 port");

    const UrlInfo opaque = UrlInfo::parse("about:blank");
    checkTrue(opaque.valid, "about: is valid");
    checkEqual(opaque.scheme, "about", "about scheme");
    checkEqual(opaque.host, "", "about has no host");
    checkTrue(!opaque.isHttpFamily(), "about is not http family");

    checkTrue(!UrlInfo::parse("example.com/path").valid, "no scheme is invalid");
    checkTrue(!UrlInfo::parse("").valid, "empty is invalid");
    checkTrue(!UrlInfo::parse("://x").valid, "empty scheme is invalid");
}

void registrableDomains()
{
    checkEqual(registrableDomainOf("example.com"), "example.com", "two labels");
    checkEqual(registrableDomainOf("cdn.assets.example.com"), "example.com", "deep subdomain");
    checkEqual(registrableDomainOf("example.co.uk"), "example.co.uk", "two-label suffix");
    checkEqual(registrableDomainOf("cdn.example.co.uk"), "example.co.uk",
               "subdomain under a two-label suffix");
    checkEqual(registrableDomainOf("localhost"), "localhost", "single label");
    checkEqual(registrableDomainOf("192.168.1.10"), "192.168.1.10", "ipv4 kept whole");
    checkEqual(registrableDomainOf("::1"), "::1", "ipv6 kept whole");
    checkEqual(registrableDomainOf(""), "", "empty host");
}

void thirdParty()
{
    const UrlInfo tracker = UrlInfo::parse("https://tracker.example.net/pixel.gif");
    const UrlInfo sameSite = UrlInfo::parse("https://cdn.example.com/app.js");

    checkTrue(isThirdParty(tracker, "example.com"), "different domain is third party");
    checkTrue(!isThirdParty(sameSite, "example.com"), "subdomain is first party");
    checkTrue(!isThirdParty(sameSite, "www.example.com"), "sibling subdomain is first party");
    checkTrue(isThirdParty(tracker, ""), "unknown first party is treated as third party");
    checkTrue(isThirdParty(UrlInfo::parse("about:blank"), "example.com"),
              "hostless request is third party");
}

} // namespace

int main()
{
    parsing();
    registrableDomains();
    thirdParty();
    return pbtest::finish();
}
