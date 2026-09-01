#include "check.h"
#include "network/core/filter_rule.h"

#include <algorithm>

using namespace pb::net;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

FilterRule parse(const char *line)
{
    const ParseResult result = parseFilterRule(line);
    checkTrue(result.outcome == ParseOutcome::Rule, line);
    return result.rule;
}

bool matches(const char *line, const char *url)
{
    return patternMatches(parse(line), url);
}

void patterns()
{
    checkTrue(matches("/ads/banner", "https://example.com/ads/banner.gif"), "substring");
    checkTrue(!matches("/ads/banner", "https://example.com/content.gif"), "substring miss");

    checkTrue(matches("||ads.example.com^", "https://ads.example.com/x.js"), "domain anchor");
    checkTrue(matches("||ads.example.com^", "https://sub.ads.example.com/x.js"),
              "domain anchor matches a subdomain boundary");
    checkTrue(!matches("||ads.example.com^", "https://notads.example.com/x.js"),
              "domain anchor does not match a partial label");
    checkTrue(!matches("||ads.example.com^", "https://example.com/ads.example.com"),
              "domain anchor does not match inside a path");

    checkTrue(matches("|https://tracker.example", "https://tracker.example/x"), "start anchor");
    checkTrue(!matches("|https://tracker.example", "https://cdn.example/?u=https://tracker.example"),
              "start anchor is not a substring match");

    checkTrue(matches("/track.gif|", "https://example.com/track.gif"), "end anchor");
    checkTrue(!matches("/track.gif|", "https://example.com/track.gif?x=1"), "end anchor miss");

    checkTrue(matches("/ad*/banner", "https://example.com/ads/banner"), "wildcard");
    checkTrue(matches("||example.com^*/pixel", "https://example.com/a/b/pixel"),
              "wildcard after separator");

    // "^" is a separator or the end of the URL.
    checkTrue(matches("||example.com^", "https://example.com"), "separator matches end of URL");
    checkTrue(matches("||example.com^", "https://example.com/"), "separator matches slash");
    checkTrue(matches("||example.com^", "https://example.com:8080/"), "separator matches colon");
    checkTrue(!matches("||example.com^", "https://example.commercial.test/"),
              "separator does not match a letter");
}

void options()
{
    const FilterRule script = parse("/x.js$script,third-party");
    checkTrue(script.appliesToType(ResourceScript), "script type included");
    checkTrue(!script.appliesToType(ResourceImage), "other types excluded");
    checkTrue(script.appliesToParty(true), "third party allowed");
    checkTrue(!script.appliesToParty(false), "first party excluded");

    const FilterRule notScript = parse("/x$~script");
    checkTrue(!notScript.appliesToType(ResourceScript), "negated type excluded");
    checkTrue(notScript.appliesToType(ResourceImage), "everything else included");

    const FilterRule domains = parse("/pixel$domain=example.com|~safe.example.com");
    checkTrue(domains.appliesToDocument("example.com"), "included domain");
    checkTrue(domains.appliesToDocument("www.example.com"), "subdomain of an included domain");
    checkTrue(!domains.appliesToDocument("safe.example.com"), "excluded subdomain");
    checkTrue(!domains.appliesToDocument("other.test"), "domain not in the include list");

    const FilterRule exception = parse("@@||example.com/api$xmlhttprequest");
    checkTrue(exception.exception, "exception parsed");
    checkTrue(exception.appliesToType(ResourceXhr), "exception type");

    checkTrue(parse("/x$important").important, "important parsed");
    checkTrue(parse("||example.com^$first-party").appliesToParty(false), "first-party option");
}

void unsupportedInputIsReportedNotGuessed()
{
    checkTrue(parseFilterRule("").outcome == ParseOutcome::Empty, "empty line");
    checkTrue(parseFilterRule("! a comment").outcome == ParseOutcome::Comment, "comment");
    checkTrue(parseFilterRule("[Adblock Plus 2.0]").outcome == ParseOutcome::Comment, "header");
    checkTrue(parseFilterRule("example.com##.ad-banner").outcome
                  == ParseOutcome::UnsupportedCosmetic,
              "cosmetic rule skipped");
    checkTrue(parseFilterRule("example.com#@#.ad").outcome == ParseOutcome::UnsupportedCosmetic,
              "cosmetic exception skipped");
    checkTrue(parseFilterRule("/banner\\d+/").outcome == ParseOutcome::UnsupportedRegex,
              "regex rule skipped");
    checkTrue(parseFilterRule("/x$csp=script-src none").outcome
                  == ParseOutcome::UnsupportedOption,
              "a rule with an unknown option is dropped rather than applied too broadly");
    checkTrue(parseFilterRule("$script").outcome == ParseOutcome::Malformed,
              "an options-only line is malformed");
}

void tokenisation()
{
    const auto tokens = extractTokens("https://ads.example.com/banner.js?id=7");
    checkTrue(std::find(tokens.begin(), tokens.end(), "ads") != tokens.end(), "host token");
    checkTrue(std::find(tokens.begin(), tokens.end(), "example") != tokens.end(), "domain token");
    checkTrue(std::find(tokens.begin(), tokens.end(), "banner") != tokens.end(), "path token");
    checkTrue(std::find(tokens.begin(), tokens.end(), "id") == tokens.end(),
              "short runs are not tokens");

    checkEqual(parse("||ads.example.com^").token, "example", "rule indexed by its longest token");
    checkEqual(parse("/a/b^").token, "", "a rule with no long token stays untokenized");
}

} // namespace

int main()
{
    patterns();
    options();
    unsupportedInputIsReportedNotGuessed();
    tokenisation();
    return pbtest::finish();
}
