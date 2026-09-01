// Minimal URL decomposition for the Qt-free core.
//
// This is not a general-purpose URL parser: Chromium already parsed and
// normalised every URL the browser acts on. UrlInfo exists so blocking,
// security and third-party decisions can be written and unit-tested without Qt.

#ifndef PB_NETWORK_CORE_URL_INFO_H
#define PB_NETWORK_CORE_URL_INFO_H

#include <string>
#include <string_view>

namespace pb::net {

struct UrlInfo {
    bool valid = false;
    std::string scheme;  // lower case, no ":"
    std::string host;    // lower case, no port, no credentials
    std::string port;    // empty when the URL used the default port
    std::string path;    // starts with "/" when present, otherwise empty
    std::string query;   // without "?"
    std::string fragment;// without "#"

    static UrlInfo parse(std::string_view url);

    bool isSecureScheme() const;   // https or wss
    bool isHttpFamily() const;     // http, https, ws, wss
    std::string hostWithPort() const;

    // Best-effort eTLD+1. Uses a compact table of common multi-label public
    // suffixes rather than the full Public Suffix List, so an unusual suffix
    // (e.g. a rarely seen ccTLD second level) can group two sites that the PSL
    // would separate. It is only used for first/third-party classification;
    // getting it wrong changes how aggressively a request is blocked, never
    // whether data leaves the machine. See docs/architecture.md.
    std::string registrableDomain() const;
};

// True when `request` belongs to a different site than the document that
// triggered it. An empty or unparsable first-party host is treated as
// third-party, which is the safer default for blocking.
bool isThirdParty(const UrlInfo &request, std::string_view firstPartyHost);

std::string registrableDomainOf(std::string_view host);

} // namespace pb::net

#endif // PB_NETWORK_CORE_URL_INFO_H
