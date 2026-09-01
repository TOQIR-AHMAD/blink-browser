#include "network/core/network_audit.h"

#include "utils/text.h"

#include <algorithm>

namespace pb::net {

const std::vector<std::string> &developerControlledHosts()
{
    // Intentionally empty. This browser has no backend of any kind: no update
    // service, no telemetry, no analytics, no crash collector, no filter
    // mirror, no sync. Adding a host here would be a privacy change requiring
    // its own review (PLAN.md §15, §29, §43, §48).
    static const std::vector<std::string> hosts = {};
    return hosts;
}

bool isDeveloperControlledHost(std::string_view host)
{
    const std::string lowered = pb::text::toLower(host);
    const auto &hosts = developerControlledHosts();
    return std::any_of(hosts.begin(), hosts.end(), [&lowered](const std::string &known) {
        return pb::text::isSubdomainOf(lowered, known);
    });
}

ConnectionClass classifyConnection(std::string_view host, std::string_view documentHost,
                                   std::string_view searchHost, std::string_view dohHost,
                                   const std::vector<std::string> &filterListHosts)
{
    if (host.empty())
        return ConnectionClass::Unknown;
    if (isDeveloperControlledHost(host))
        return ConnectionClass::DeveloperControlled;

    const std::string lowered = pb::text::toLower(host);

    if (!dohHost.empty() && pb::text::isSubdomainOf(lowered, pb::text::toLower(dohHost)))
        return ConnectionClass::UserSelectedService;
    if (!searchHost.empty() && pb::text::isSubdomainOf(lowered, pb::text::toLower(searchHost)))
        return ConnectionClass::UserSelectedService;
    for (const std::string &filterHost : filterListHosts) {
        if (pb::text::isSubdomainOf(lowered, pb::text::toLower(filterHost)))
            return ConnectionClass::FilterListUpdate;
    }
    if (!documentHost.empty())
        return ConnectionClass::WebsiteRequest;
    return ConnectionClass::Unknown;
}

void NetworkAudit::record(std::string_view host, ConnectionClass classification, bool blocked)
{
    if (host.empty())
        return;

    const std::string key = pb::text::toLower(host);
    AuditRow &row = m_rows[key];
    if (row.host.empty()) {
        row.host = key;
        row.classification = classification;
    }
    ++row.requests;
    if (blocked)
        ++row.blocked;

    ++m_total;
    if (classification == ConnectionClass::DeveloperControlled)
        m_developerControlled = true;
}

std::vector<AuditRow> NetworkAudit::rows() const
{
    std::vector<AuditRow> result;
    result.reserve(m_rows.size());
    for (const auto &entry : m_rows)
        result.push_back(entry.second);
    std::sort(result.begin(), result.end(), [](const AuditRow &a, const AuditRow &b) {
        return a.requests > b.requests;
    });
    return result;
}

void NetworkAudit::clear()
{
    m_rows.clear();
    m_total = 0;
    m_developerControlled = false;
}

} // namespace pb::net
