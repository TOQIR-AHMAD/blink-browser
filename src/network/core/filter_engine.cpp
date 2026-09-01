#include "network/core/filter_engine.h"

#include "utils/text.h"

namespace pb::net {

void FilterEngine::insert(Index &index, FilterRule rule)
{
    if (rule.token.empty())
        index.untokenized.push_back(std::move(rule));
    else
        index.byToken[rule.token].push_back(std::move(rule));
}

LoadReport FilterEngine::addRules(std::string_view listText, RuleCategory category)
{
    LoadReport report;

    std::size_t start = 0;
    while (start <= listText.size()) {
        std::size_t end = listText.find('\n', start);
        if (end == std::string_view::npos)
            end = listText.size();
        const std::string_view line = listText.substr(start, end - start);
        start = end + 1;

        const ParseResult parsed = parseFilterRule(line, category);
        switch (parsed.outcome) {
        case ParseOutcome::Rule:
            if (parsed.rule.exception) {
                ++m_exceptionCount;
                insert(m_exceptions, parsed.rule);
            } else {
                ++m_ruleCount;
                insert(m_blocking, parsed.rule);
            }
            ++report.rules;
            break;
        case ParseOutcome::Comment:
            ++report.comments;
            break;
        case ParseOutcome::Empty:
            break;
        case ParseOutcome::UnsupportedCosmetic:
            ++report.cosmeticSkipped;
            break;
        case ParseOutcome::UnsupportedRegex:
            ++report.regexSkipped;
            break;
        case ParseOutcome::UnsupportedOption:
            ++report.optionSkipped;
            break;
        case ParseOutcome::Malformed:
            ++report.malformed;
            break;
        }

        if (end == listText.size())
            break;
    }

    return report;
}

void FilterEngine::clear()
{
    m_blocking = Index{};
    m_exceptions = Index{};
    m_ruleCount = 0;
    m_exceptionCount = 0;
}

bool FilterEngine::ruleMatches(const FilterRule &rule, const Request &request,
                               const std::string &url)
{
    if (!rule.appliesToType(request.type))
        return false;
    if (!rule.appliesToParty(request.thirdParty))
        return false;
    if (!rule.appliesToDocument(request.documentHost))
        return false;
    return patternMatches(rule, url);
}

bool FilterEngine::matchesIn(const Index &index, const std::vector<std::string> &tokens,
                             const Request &request, const std::string &url, bool importantOnly,
                             MatchResult *result)
{
    const auto check = [&](const std::vector<FilterRule> &rules) {
        for (const FilterRule &rule : rules) {
            if (importantOnly && !rule.important)
                continue;
            if (!ruleMatches(rule, request, url))
                continue;
            if (result) {
                result->blocked = !rule.exception;
                result->category = rule.category;
                result->rulePattern = rule.pattern;
            }
            return true;
        }
        return false;
    };

    for (const std::string &token : tokens) {
        const auto it = index.byToken.find(token);
        if (it != index.byToken.end() && check(it->second))
            return true;
    }
    return check(index.untokenized);
}

MatchResult FilterEngine::match(const Request &request) const
{
    MatchResult result;
    if (request.url.empty())
        return result;

    const std::string url = pb::text::toLower(request.url);
    const std::vector<std::string> tokens = extractTokens(url);

    // An $important block rule beats an exception; that is the whole point of
    // the option, and filter lists rely on it.
    MatchResult important;
    if (matchesIn(m_blocking, tokens, request, url, true, &important))
        return important;

    MatchResult blocking;
    if (!matchesIn(m_blocking, tokens, request, url, false, &blocking))
        return result;

    MatchResult exception;
    if (matchesIn(m_exceptions, tokens, request, url, false, &exception))
        return result; // allowed by an exception rule

    return blocking;
}

} // namespace pb::net
