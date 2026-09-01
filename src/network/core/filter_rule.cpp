#include "network/core/filter_rule.h"

#include "utils/text.h"

#include <algorithm>

namespace pb::net {
namespace {

bool isSeparator(char c)
{
    const bool wordChar = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == '%';
    return !wordChar;
}

bool isTokenChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

std::uint32_t typeFromOption(std::string_view name)
{
    if (name == "script")
        return ResourceScript;
    if (name == "image")
        return ResourceImage;
    if (name == "stylesheet" || name == "css")
        return ResourceStylesheet;
    if (name == "font")
        return ResourceFont;
    if (name == "media")
        return ResourceMedia;
    if (name == "xmlhttprequest" || name == "xhr")
        return ResourceXhr;
    if (name == "subdocument" || name == "frame")
        return ResourceSubdocument;
    if (name == "document" || name == "doc")
        return ResourceDocument;
    if (name == "websocket")
        return ResourceWebSocket;
    if (name == "ping" || name == "beacon")
        return ResourcePing;
    if (name == "other")
        return ResourceOther;
    return 0;
}

// Options that only matter to a cosmetic or scriptlet engine, or that this
// engine treats as no-ops. Accepting them keeps otherwise usable network rules
// from being thrown away.
bool isIgnorableOption(std::string_view name)
{
    return name == "popup" || name == "popunder" || name == "object"
        || name == "object-subrequest" || name == "elemhide" || name == "generichide"
        || name == "genericblock" || name == "collapse" || name == "webrtc"
        || name == "inline-script" || name == "inline-font" || name == "all";
}

bool matchAt(const std::string &url, std::size_t urlPos, const std::string &pattern,
             std::size_t patternPos, bool endAnchor)
{
    while (patternPos < pattern.size()) {
        const char pc = pattern[patternPos];

        if (pc == '*') {
            while (patternPos < pattern.size() && pattern[patternPos] == '*')
                ++patternPos;
            if (patternPos == pattern.size())
                return true;
            for (std::size_t at = urlPos; at <= url.size(); ++at) {
                if (matchAt(url, at, pattern, patternPos, endAnchor))
                    return true;
            }
            return false;
        }

        if (pc == '^') {
            // A separator, or the end of the URL.
            if (urlPos == url.size())
                return patternPos + 1 == pattern.size();
            if (!isSeparator(url[urlPos]))
                return false;
            ++urlPos;
            ++patternPos;
            continue;
        }

        if (urlPos >= url.size() || url[urlPos] != pc)
            return false;
        ++urlPos;
        ++patternPos;
    }

    return endAnchor ? urlPos == url.size() : true;
}

// Positions in the URL where a "||" anchored pattern may start: the beginning
// of the host, and every label boundary inside it.
std::vector<std::size_t> domainAnchorPositions(const std::string &url)
{
    std::vector<std::size_t> positions;
    const std::size_t schemeEnd = url.find("://");
    const std::size_t hostStart = schemeEnd == std::string::npos ? 0 : schemeEnd + 3;
    if (hostStart >= url.size())
        return positions;

    positions.push_back(hostStart);
    for (std::size_t i = hostStart; i < url.size(); ++i) {
        const char c = url[i];
        if (c == '/' || c == '?' || c == '#')
            break;
        if (c == '.' && i + 1 < url.size())
            positions.push_back(i + 1);
    }
    return positions;
}

} // namespace

std::vector<std::string> extractTokens(std::string_view text)
{
    std::vector<std::string> tokens;
    std::string current;
    for (const char c : text) {
        if (isTokenChar(c)) {
            current.push_back(c);
            continue;
        }
        if (current.size() >= 3)
            tokens.push_back(current);
        current.clear();
    }
    if (current.size() >= 3)
        tokens.push_back(current);
    return tokens;
}

bool FilterRule::appliesToType(std::uint32_t type) const
{
    return (typeMask & type) != 0;
}

bool FilterRule::appliesToParty(bool thirdParty) const
{
    switch (party) {
    case PartyRestriction::Any:
        return true;
    case PartyRestriction::FirstPartyOnly:
        return !thirdParty;
    case PartyRestriction::ThirdPartyOnly:
        return thirdParty;
    }
    return true;
}

bool FilterRule::appliesToDocument(std::string_view documentHost) const
{
    const auto matches = [documentHost](const std::string &domain) {
        return pb::text::isSubdomainOf(documentHost, domain);
    };

    if (std::any_of(excludeDomains.begin(), excludeDomains.end(), matches))
        return false;
    if (includeDomains.empty())
        return true;
    return std::any_of(includeDomains.begin(), includeDomains.end(), matches);
}

bool patternMatches(const FilterRule &rule, const std::string &url)
{
    const std::string &pattern = rule.pattern;
    if (pattern.empty())
        return false;

    if (rule.domainAnchor) {
        for (const std::size_t position : domainAnchorPositions(url)) {
            if (matchAt(url, position, pattern, 0, rule.endAnchor))
                return true;
        }
        return false;
    }

    if (rule.startAnchor)
        return matchAt(url, 0, pattern, 0, rule.endAnchor);

    for (std::size_t position = 0; position <= url.size(); ++position) {
        if (matchAt(url, position, pattern, 0, rule.endAnchor))
            return true;
    }
    return false;
}

ParseResult parseFilterRule(std::string_view line, RuleCategory category)
{
    ParseResult result;
    const std::string_view trimmed = pb::text::trim(line);

    if (trimmed.empty()) {
        result.outcome = ParseOutcome::Empty;
        return result;
    }
    if (trimmed.front() == '!' || trimmed.front() == '[') {
        result.outcome = ParseOutcome::Comment;
        return result;
    }
    // Cosmetic rules: element hiding, scriptlet injection and their exceptions.
    if (trimmed.find("##") != std::string_view::npos
        || trimmed.find("#@#") != std::string_view::npos
        || trimmed.find("#?#") != std::string_view::npos
        || trimmed.find("#$#") != std::string_view::npos) {
        result.outcome = ParseOutcome::UnsupportedCosmetic;
        return result;
    }

    std::string_view body = trimmed;
    FilterRule rule;
    rule.category = category;

    if (pb::text::startsWith(body, "@@")) {
        rule.exception = true;
        body.remove_prefix(2);
    }

    // Options.
    const std::size_t optionsAt = body.rfind('$');
    if (optionsAt != std::string_view::npos && optionsAt + 1 < body.size()) {
        const std::string_view optionText = body.substr(optionsAt + 1);
        body = body.substr(0, optionsAt);

        std::uint32_t includedTypes = 0;
        std::uint32_t excludedTypes = 0;
        for (std::string_view option : pb::text::split(optionText, ',')) {
            option = pb::text::trim(option);
            if (option.empty())
                continue;

            bool negated = false;
            if (option.front() == '~') {
                negated = true;
                option.remove_prefix(1);
            }

            if (pb::text::startsWith(option, "domain=")) {
                for (std::string_view domain : pb::text::split(option.substr(7), '|')) {
                    if (domain.empty())
                        continue;
                    if (domain.front() == '~')
                        rule.excludeDomains.push_back(pb::text::toLower(domain.substr(1)));
                    else
                        rule.includeDomains.push_back(pb::text::toLower(domain));
                }
                continue;
            }
            if (option == "third-party" || option == "3p") {
                rule.party = negated ? PartyRestriction::FirstPartyOnly
                                     : PartyRestriction::ThirdPartyOnly;
                continue;
            }
            if (option == "first-party" || option == "1p") {
                rule.party = negated ? PartyRestriction::ThirdPartyOnly
                                     : PartyRestriction::FirstPartyOnly;
                continue;
            }
            if (option == "important") {
                rule.important = true;
                continue;
            }
            if (option == "match-case") {
                rule.matchCase = true;
                continue;
            }
            if (const std::uint32_t type = typeFromOption(option); type != 0) {
                if (negated)
                    excludedTypes |= type;
                else
                    includedTypes |= type;
                continue;
            }
            if (isIgnorableOption(option))
                continue;

            // An option we do not understand could change what the rule means,
            // so the rule is dropped rather than applied too broadly.
            result.outcome = ParseOutcome::UnsupportedOption;
            return result;
        }

        if (includedTypes != 0)
            rule.typeMask = includedTypes;
        if (excludedTypes != 0)
            rule.typeMask &= ~excludedTypes;
        if (rule.typeMask == 0) {
            result.outcome = ParseOutcome::Malformed;
            return result;
        }
    }

    if (body.size() >= 2 && body.front() == '/' && body.back() == '/') {
        result.outcome = ParseOutcome::UnsupportedRegex;
        return result;
    }

    if (pb::text::startsWith(body, "||")) {
        rule.domainAnchor = true;
        body.remove_prefix(2);
    } else if (pb::text::startsWith(body, "|")) {
        rule.startAnchor = true;
        body.remove_prefix(1);
    }
    if (!body.empty() && body.back() == '|') {
        rule.endAnchor = true;
        body.remove_suffix(1);
    }

    if (body.empty()) {
        result.outcome = ParseOutcome::Malformed;
        return result;
    }

    rule.pattern = rule.matchCase ? std::string(body) : pb::text::toLower(body);

    // Index the rule under its longest token: the more specific the token, the
    // fewer candidate rules a request has to be checked against.
    const auto tokens = extractTokens(rule.pattern);
    if (!tokens.empty()) {
        const auto longest = std::max_element(
            tokens.begin(), tokens.end(),
            [](const std::string &a, const std::string &b) { return a.size() < b.size(); });
        rule.token = *longest;
    }

    result.outcome = ParseOutcome::Rule;
    result.rule = std::move(rule);
    return result;
}

} // namespace pb::net
