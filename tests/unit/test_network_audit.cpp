// PLAN.md §43: the browser must never silently contact a server the
// developers control. This test is the enforcement.

#include "check.h"
#include "network/core/network_audit.h"

using namespace pb::net;
using pbtest::checkEqual;
using pbtest::checkTrue;

int main()
{
    checkTrue(developerControlledHosts().empty(),
              "there are no developer-controlled hosts, and adding one must fail this test");
    checkTrue(!isDeveloperControlledHost("example.com"), "no host is developer-controlled");

    const std::vector<std::string> filterHosts = { "easylist.to" };

    checkTrue(classifyConnection("example.com", "example.com", "duckduckgo.com", "", filterHosts)
                  == ConnectionClass::WebsiteRequest,
              "a page request is a website request");
    checkTrue(classifyConnection("cdn.example.net", "example.com", "duckduckgo.com", "",
                                 filterHosts)
                  == ConnectionClass::WebsiteRequest,
              "a third-party resource is still a website request");
    checkTrue(classifyConnection("duckduckgo.com", "", "duckduckgo.com", "", filterHosts)
                  == ConnectionClass::UserSelectedService,
              "the configured search provider is a user-selected service");
    checkTrue(classifyConnection("dns.example", "", "duckduckgo.com", "dns.example", filterHosts)
                  == ConnectionClass::UserSelectedService,
              "the configured DoH resolver is a user-selected service");
    checkTrue(classifyConnection("easylist.to", "", "duckduckgo.com", "", filterHosts)
                  == ConnectionClass::FilterListUpdate,
              "a filter list host is a filter update");
    checkTrue(classifyConnection("", "example.com", "", "", filterHosts)
                  == ConnectionClass::Unknown,
              "an empty host is unknown");

    NetworkAudit audit;
    audit.record("example.com", ConnectionClass::WebsiteRequest, false);
    audit.record("example.com", ConnectionClass::WebsiteRequest, false);
    audit.record("ads.example.net", ConnectionClass::WebsiteRequest, true);

    checkEqual(static_cast<long long>(audit.totalRequests()), 3, "requests counted");
    const auto rows = audit.rows();
    checkEqual(static_cast<long long>(rows.size()), 2, "aggregated by host");
    checkEqual(rows.front().host, "example.com", "most-requested host first");
    checkEqual(static_cast<long long>(rows.front().requests), 2, "request count per host");
    checkEqual(static_cast<long long>(rows.back().blocked), 1, "blocked count per host");
    checkTrue(!audit.sawDeveloperControlledHost(), "no developer host was contacted");

    audit.clear();
    checkEqual(static_cast<long long>(audit.totalRequests()), 0, "audit clears");

    return pbtest::finish();
}
