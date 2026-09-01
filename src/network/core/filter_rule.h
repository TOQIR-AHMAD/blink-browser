// One line of an Adblock-Plus-syntax filter list.
//
// PLAN.md §20 says to stay compatible with existing filter lists rather than
// inventing a filtering language. This parser therefore implements the network
// filtering subset of the ABP syntax - the part that decides whether a request
// is made at all:
//
//   ||example.com^            domain-anchored block
//   |http://example.com       start-anchored block
//   /ads/banner.gif           substring block
//   @@||example.com/api       exception
//   $script,third-party       resource type and party options
//   $domain=a.com|~b.com      document-domain restrictions
//   $important                wins over exceptions
//
// Deliberately not supported, and reported as skipped rather than silently
// ignored: cosmetic rules (##, #@#, #?#), which hide page elements rather than
// block requests, and regular-expression rules (/.../), whose evaluation cost
// is unbounded. Both are counted so the UI can say how much of a list is in
// use instead of implying all of it is.

#ifndef PB_NETWORK_CORE_FILTER_RULE_H
#define PB_NETWORK_CORE_FILTER_RULE_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace pb::net {

enum ResourceType : std::uint32_t {
    ResourceDocument = 1u << 0,
    ResourceSubdocument = 1u << 1,
    ResourceScript = 1u << 2,
    ResourceStylesheet = 1u << 3,
    ResourceImage = 1u << 4,
    ResourceFont = 1u << 5,
    ResourceMedia = 1u << 6,
    ResourceXhr = 1u << 7,
    ResourceWebSocket = 1u << 8,
    ResourcePing = 1u << 9,
    ResourceOther = 1u << 10,
    ResourceAll = (1u << 11) - 1,
};

enum class PartyRestriction {
    Any,
    FirstPartyOnly,
    ThirdPartyOnly,
};

// Which list a rule came from, so the dashboard can separate "trackers
// blocked" from "ads blocked" without guessing.
enum class RuleCategory {
    Ads,
    Trackers,
    Other,
};

struct FilterRule {
    std::string pattern;
    bool exception = false;
    bool important = false;
    bool domainAnchor = false; // ||
    bool startAnchor = false;  // leading |
    bool endAnchor = false;    // trailing |
    bool matchCase = false;
    std::uint32_t typeMask = ResourceAll;
    PartyRestriction party = PartyRestriction::Any;
    std::vector<std::string> includeDomains;
    std::vector<std::string> excludeDomains;
    RuleCategory category = RuleCategory::Other;

    // The token the engine indexes this rule under. Empty means the rule has
    // no usable token and must be checked against every request.
    std::string token;

    bool appliesToDocument(std::string_view documentHost) const;
    bool appliesToType(std::uint32_t type) const;
    bool appliesToParty(bool thirdParty) const;
};

enum class ParseOutcome {
    Rule,
    Comment,          // "!" or "[Adblock ...]" header
    Empty,
    UnsupportedCosmetic,
    UnsupportedRegex,
    UnsupportedOption, // a $option this engine does not implement
    Malformed,
};

struct ParseResult {
    ParseOutcome outcome = ParseOutcome::Malformed;
    FilterRule rule;
};

ParseResult parseFilterRule(std::string_view line, RuleCategory category = RuleCategory::Other);

// Matches a rule's pattern against a URL. `url` must already be lower-cased
// unless the rule is match-case.
bool patternMatches(const FilterRule &rule, const std::string &url);

// Extracts the indexable tokens of a URL: runs of at least three alphanumeric
// characters. Used by both the parser (to pick a rule's token) and the engine
// (to find candidate rules).
std::vector<std::string> extractTokens(std::string_view text);

} // namespace pb::net

#endif // PB_NETWORK_CORE_FILTER_RULE_H
