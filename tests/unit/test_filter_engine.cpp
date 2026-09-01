#include "check.h"
#include "network/core/filter_engine.h"

using namespace pb::net;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

constexpr char kList[] = R"(! Test list
||ads.example.net^
||tracker.test^$third-party
/banner.gif
||cdn.example.net/beacon^$image
@@||ads.example.net/allowed^
||blocked.test^$important
@@||blocked.test^
example.com##.ad
/regex\d+/
)";

Request request(const char *url, const char *documentHost, std::uint32_t type = ResourceOther,
                bool thirdParty = true)
{
    Request value;
    value.url = url;
    value.documentHost = documentHost;
    value.type = type;
    value.thirdParty = thirdParty;
    return value;
}

void loading()
{
    FilterEngine engine;
    const LoadReport report = engine.addRules(kList, RuleCategory::Trackers);

    checkEqual(static_cast<long long>(report.rules), 7, "rules loaded");
    checkEqual(static_cast<long long>(report.comments), 1, "comments counted");
    checkEqual(static_cast<long long>(report.cosmeticSkipped), 1, "cosmetic rules counted");
    checkEqual(static_cast<long long>(report.regexSkipped), 1, "regex rules counted");
    checkEqual(static_cast<long long>(engine.ruleCount()), 5, "blocking rules");
    checkEqual(static_cast<long long>(engine.exceptionCount()), 2, "exception rules");
}

void matching()
{
    FilterEngine engine;
    engine.addRules(kList, RuleCategory::Trackers);

    checkTrue(engine.match(request("https://ads.example.net/pixel.gif", "example.com")).blocked,
              "domain rule blocks");
    checkTrue(!engine.match(request("https://safe.example.net/pixel.gif", "example.com")).blocked,
              "unrelated host allowed");

    checkTrue(engine.match(request("https://example.com/banner.gif", "example.com")).blocked,
              "substring rule blocks");

    checkTrue(engine.match(request("https://tracker.test/t.js", "example.com", ResourceScript,
                                   true))
                  .blocked,
              "third-party rule blocks a third-party request");
    checkTrue(!engine.match(request("https://tracker.test/t.js", "tracker.test", ResourceScript,
                                    false))
                   .blocked,
              "third-party rule ignores a first-party request");

    // "^" matches a separator, and "." is not one - so the rule below covers
    // "/beacon/..." and "/beacon?..." but not "/beacon.png".
    checkTrue(engine.match(request("https://cdn.example.net/beacon/pixel.png", "example.com",
                                   ResourceImage))
                  .blocked,
              "type-restricted rule blocks its type");
    checkTrue(!engine.match(request("https://cdn.example.net/beacon/pixel.js", "example.com",
                                    ResourceScript))
                   .blocked,
              "type-restricted rule ignores other types");
    checkTrue(!engine.match(request("https://cdn.example.net/beacon.png", "example.com",
                                    ResourceImage))
                   .blocked,
              "a dot is not a separator, so \"beacon^\" does not match \"beacon.png\"");
}

void exceptionsAndImportant()
{
    FilterEngine engine;
    engine.addRules(kList, RuleCategory::Trackers);

    checkTrue(engine.match(request("https://ads.example.net/blocked.js", "example.com")).blocked,
              "blocked without an exception");
    checkTrue(!engine.match(request("https://ads.example.net/allowed/app.js", "example.com"))
                   .blocked,
              "an exception rule wins over a block rule");
    checkTrue(engine.match(request("https://blocked.test/x", "example.com")).blocked,
              "an $important block wins over an exception");
}

void categoriesAreKept()
{
    FilterEngine engine;
    engine.addRules("||adserver.test^\n", RuleCategory::Ads);
    engine.addRules("||analytics.test^\n", RuleCategory::Trackers);

    const MatchResult ad = engine.match(request("https://adserver.test/a", "example.com"));
    checkTrue(ad.blocked && ad.category == RuleCategory::Ads, "ad category preserved");

    const MatchResult tracker = engine.match(request("https://analytics.test/a", "example.com"));
    checkTrue(tracker.blocked && tracker.category == RuleCategory::Trackers,
              "tracker category preserved");
}

void documentRestrictions()
{
    FilterEngine engine;
    engine.addRules("/widget.js$domain=news.test|~sport.news.test\n", RuleCategory::Other);

    checkTrue(engine.match(request("https://cdn.test/widget.js", "news.test")).blocked,
              "blocked on the listed document domain");
    checkTrue(!engine.match(request("https://cdn.test/widget.js", "sport.news.test")).blocked,
              "not blocked on an excluded subdomain");
    checkTrue(!engine.match(request("https://cdn.test/widget.js", "other.test")).blocked,
              "not blocked elsewhere");
}

void emptyEngineBlocksNothing()
{
    FilterEngine engine;
    checkTrue(!engine.match(request("https://ads.example.net/x", "example.com")).blocked,
              "an engine with no rules blocks nothing");

    engine.addRules("||ads.example.net^\n", RuleCategory::Ads);
    checkTrue(engine.match(request("https://ads.example.net/x", "example.com")).blocked,
              "rules take effect");
    engine.clear();
    checkTrue(!engine.match(request("https://ads.example.net/x", "example.com")).blocked,
              "clear removes every rule");
    checkEqual(static_cast<long long>(engine.ruleCount()), 0, "rule count reset");
}

void caseAndQueryHandling()
{
    FilterEngine engine;
    engine.addRules("||ADS.example.net^\n", RuleCategory::Ads);
    checkTrue(engine.match(request("https://ads.EXAMPLE.net/Pixel.GIF", "example.com")).blocked,
              "matching is case-insensitive by default");
}

} // namespace

int main()
{
    loading();
    matching();
    exceptionsAndImportant();
    categoriesAreKept();
    documentRestrictions();
    emptyEngineBlocksNothing();
    caseAndQueryHandling();
    return pbtest::finish();
}
