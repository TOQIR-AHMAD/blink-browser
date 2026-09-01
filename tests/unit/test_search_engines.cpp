#include "check.h"
#include "settings/core/search_engines.h"

using namespace pb::settings;
using pbtest::checkEqual;
using pbtest::checkTrue;

int main()
{
    const auto &engines = builtInSearchEngines();
    checkTrue(engines.size() >= 5, "the plan's providers are all offered");
    checkTrue(findSearchEngine("duckduckgo") != nullptr, "duckduckgo present");
    checkTrue(findSearchEngine("startpage") != nullptr, "startpage present");
    checkTrue(findSearchEngine("brave") != nullptr, "brave present");
    checkTrue(findSearchEngine("google") != nullptr, "google present");
    checkTrue(findSearchEngine("bing") != nullptr, "bing present");
    checkTrue(findSearchEngine("not-a-provider") == nullptr, "unknown id returns nothing");
    checkEqual(std::string(defaultSearchEngineId()), "duckduckgo", "default provider");

    for (const SearchEngine &engine : engines) {
        checkTrue(isValidSearchTemplate(engine.queryTemplate),
                  ("built-in template is valid: " + engine.id).c_str());
        checkTrue(engine.queryTemplate.rfind("https://", 0) == 0,
                  ("built-in provider uses https: " + engine.id).c_str());
    }

    checkEqual(buildSearchUrl("https://example.com/?q={searchTerms}", "hello world"),
               "https://example.com/?q=hello%20world", "query is percent-encoded");
    checkEqual(buildSearchUrl("https://example.com/?q={searchTerms}", "a&b=c#d"),
               "https://example.com/?q=a%26b%3Dc%23d", "separators cannot escape the query");
    checkEqual(buildSearchUrl("https://example.com/?q={searchTerms}", "caf\xC3\xA9"),
               "https://example.com/?q=caf%C3%A9", "utf-8 bytes are encoded");

    checkTrue(!isValidSearchTemplate("https://example.com/?q=fixed"), "template needs the token");
    checkTrue(!isValidSearchTemplate("http://example.com/?q={searchTerms}"),
              "a plain-http template would send the query in the clear");
    checkEqual(buildSearchUrl("http://example.com/?q={searchTerms}", "x"), "",
               "an invalid template produces nothing, not a half-formed URL");

    return pbtest::finish();
}
