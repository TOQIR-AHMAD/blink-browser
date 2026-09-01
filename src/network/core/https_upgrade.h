// HTTPS-first navigation (PLAN.md §24).
//
// A top-level http:// navigation is retried as https:// first. If that fails,
// the host is remembered - in memory, for this session - so the user is not
// made to wait for a doomed https attempt on every visit, and the plain-http
// load proceeds with the address bar showing it as insecure.
//
// Sub-resources are not upgraded here: Chromium's own mixed-content rules
// already handle those, and rewriting them would break sites in ways the user
// cannot see or undo.

#ifndef PB_NETWORK_CORE_HTTPS_UPGRADE_H
#define PB_NETWORK_CORE_HTTPS_UPGRADE_H

#include "network/core/url_info.h"

#include <set>
#include <string>
#include <string_view>

namespace pb::net {

class HttpsUpgrade
{
public:
    // Returns the https URL to try instead, or an empty string when the
    // request should proceed as-is.
    std::string upgradedUrl(const UrlInfo &url, bool isMainFrame) const;

    // Called when an https attempt for a host failed to connect.
    void recordFailure(std::string_view host);
    bool isKnownHttpOnly(std::string_view host) const;

    void clear();
    std::size_t knownHttpOnlyCount() const { return m_httpOnlyHosts.size(); }

private:
    std::set<std::string, std::less<>> m_httpOnlyHosts;
};

// Hosts that are never upgraded: loopback and private addresses, where https
// usually does not exist and the traffic never leaves the machine or network.
bool isLocalHost(std::string_view host);

} // namespace pb::net

#endif // PB_NETWORK_CORE_HTTPS_UPGRADE_H
