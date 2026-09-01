// Development-only record of where the browser connects (PLAN.md §43).
//
// The audit answers one question: does this browser ever contact a server the
// developers control? The answer must be no, and the list of such hosts below
// is empty because there are none - no update service, no telemetry endpoint,
// no filter-list mirror, no crash collector. The tests assert that the list
// stays empty, which turns "we don't do that" into something a build can fail
// over.
//
// The recorder is compiled in only when PB_NETWORK_AUDIT is defined, and even
// then it aggregates by host rather than storing URLs.

#ifndef PB_NETWORK_CORE_NETWORK_AUDIT_H
#define PB_NETWORK_CORE_NETWORK_AUDIT_H

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace pb::net {

enum class ConnectionClass {
    WebsiteRequest,       // the site the user is looking at, or its resources
    UserSelectedService,  // search provider, DoH resolver, proxy: user's choice
    FilterListUpdate,     // an upstream filter list, only on an explicit update
    BrowserInfrastructure,// Chromium itself
    DeveloperControlled,  // must never occur
    Unknown,
};

struct AuditRow {
    std::string host;
    ConnectionClass classification = ConnectionClass::Unknown;
    std::size_t requests = 0;
    std::size_t blocked = 0;
};

// The set of hosts this project's developers control. It is empty, and
// tests/unit/test_network_audit.cpp fails the build if it ever is not.
const std::vector<std::string> &developerControlledHosts();
bool isDeveloperControlledHost(std::string_view host);

ConnectionClass classifyConnection(std::string_view host, std::string_view documentHost,
                                   std::string_view searchHost, std::string_view dohHost,
                                   const std::vector<std::string> &filterListHosts);

class NetworkAudit
{
public:
    void record(std::string_view host, ConnectionClass classification, bool blocked);
    std::vector<AuditRow> rows() const;
    std::size_t totalRequests() const { return m_total; }
    bool sawDeveloperControlledHost() const { return m_developerControlled; }
    void clear();

private:
    std::map<std::string, AuditRow> m_rows;
    std::size_t m_total = 0;
    bool m_developerControlled = false;
};

} // namespace pb::net

#endif // PB_NETWORK_CORE_NETWORK_AUDIT_H
