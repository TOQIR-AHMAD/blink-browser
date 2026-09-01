// Search providers.
//
// PLAN.md §50: no developer-controlled search engine, nothing hard-coded as
// mandatory, and the user picks. The default is DuckDuckGo because it is the
// listed provider that does not require an account and does not personalise
// results by identity - but it is a default, not a lock-in.
//
// Suggestions are a separate, opt-in setting: every keystroke would otherwise
// go to the provider (PLAN.md §10, §50).

#ifndef PB_SETTINGS_CORE_SEARCH_ENGINES_H
#define PB_SETTINGS_CORE_SEARCH_ENGINES_H

#include <string>
#include <string_view>
#include <vector>

namespace pb::settings {

struct SearchEngine {
    std::string id;
    std::string name;
    // Must contain the {searchTerms} placeholder.
    std::string queryTemplate;
    // Endpoint used only when the user opts into suggestions. Empty when the
    // provider has none the browser can use.
    std::string suggestionTemplate;
};

const std::vector<SearchEngine> &builtInSearchEngines();

// Returns nullptr when the id is unknown.
const SearchEngine *findSearchEngine(std::string_view id);

// The id used when nothing is configured.
std::string_view defaultSearchEngineId();

// A custom template must be https and must contain {searchTerms}: anything
// else would either leak the query in the clear or silently drop it.
bool isValidSearchTemplate(std::string_view templateText);

// Substitutes the percent-encoded query into the template. Returns an empty
// string for an invalid template rather than producing a half-formed URL.
std::string buildSearchUrl(std::string_view templateText, std::string_view query);

} // namespace pb::settings

#endif // PB_SETTINGS_CORE_SEARCH_ENGINES_H
