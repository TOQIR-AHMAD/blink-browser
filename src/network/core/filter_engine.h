// Decides whether a request is blocked.
//
// Rules are indexed by their longest token so a request only has to be checked
// against the handful of rules that share a token with its URL, plus the few
// rules that have no usable token. That is what keeps blocking cheap enough to
// run on Chromium's network thread (PLAN.md §21).
//
// The engine is a pure function of (rules, request): no state per site, no
// counters, no logging. Everything it learns about a request dies with the
// call, which is what keeps the blocker from becoming a tracker itself.

#ifndef PB_NETWORK_CORE_FILTER_ENGINE_H
#define PB_NETWORK_CORE_FILTER_ENGINE_H

#include "network/core/filter_rule.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pb::net {

struct LoadReport {
    std::size_t rules = 0;
    std::size_t comments = 0;
    std::size_t cosmeticSkipped = 0;
    std::size_t regexSkipped = 0;
    std::size_t optionSkipped = 0;
    std::size_t malformed = 0;

    std::size_t skipped() const
    {
        return cosmeticSkipped + regexSkipped + optionSkipped + malformed;
    }
};

struct Request {
    std::string url;          // absolute, as Chromium reports it
    std::string documentHost; // host of the page that caused the request
    std::uint32_t type = ResourceOther;
    bool thirdParty = true;
};

struct MatchResult {
    bool blocked = false;
    RuleCategory category = RuleCategory::Other;
    // The matched rule's pattern. For diagnostics in developer builds only:
    // it is a filter-list line, never anything derived from the user.
    std::string rulePattern;
};

class FilterEngine
{
public:
    LoadReport addRules(std::string_view listText, RuleCategory category);
    void clear();

    MatchResult match(const Request &request) const;

    std::size_t ruleCount() const { return m_ruleCount; }
    std::size_t exceptionCount() const { return m_exceptionCount; }

private:
    struct Index {
        std::unordered_map<std::string, std::vector<FilterRule>> byToken;
        std::vector<FilterRule> untokenized;
    };

    static bool matchesIn(const Index &index, const std::vector<std::string> &tokens,
                          const Request &request, const std::string &url, bool importantOnly,
                          MatchResult *result);
    static bool ruleMatches(const FilterRule &rule, const Request &request,
                            const std::string &url);
    static void insert(Index &index, FilterRule rule);

    Index m_blocking;
    Index m_exceptions;
    std::size_t m_ruleCount = 0;
    std::size_t m_exceptionCount = 0;
};

} // namespace pb::net

#endif // PB_NETWORK_CORE_FILTER_ENGINE_H
