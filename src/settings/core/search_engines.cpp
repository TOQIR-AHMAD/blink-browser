#include "settings/core/search_engines.h"

#include "utils/text.h"

namespace pb::settings {
namespace {

constexpr std::string_view kPlaceholder = "{searchTerms}";

} // namespace

const std::vector<SearchEngine> &builtInSearchEngines()
{
    static const std::vector<SearchEngine> engines = {
        { "duckduckgo", "DuckDuckGo", "https://duckduckgo.com/?q={searchTerms}",
          "https://duckduckgo.com/ac/?q={searchTerms}&type=list" },
        { "startpage", "Startpage", "https://www.startpage.com/sp/search?query={searchTerms}",
          "" },
        { "brave", "Brave Search", "https://search.brave.com/search?q={searchTerms}",
          "https://search.brave.com/api/suggest?q={searchTerms}" },
        { "google", "Google", "https://www.google.com/search?q={searchTerms}",
          "https://suggestqueries.google.com/complete/search?client=firefox&q={searchTerms}" },
        { "bing", "Bing", "https://www.bing.com/search?q={searchTerms}",
          "https://api.bing.com/osjson.aspx?query={searchTerms}" },
    };
    return engines;
}

const SearchEngine *findSearchEngine(std::string_view id)
{
    for (const SearchEngine &engine : builtInSearchEngines()) {
        if (engine.id == id)
            return &engine;
    }
    return nullptr;
}

std::string_view defaultSearchEngineId()
{
    return "duckduckgo";
}

bool isValidSearchTemplate(std::string_view templateText)
{
    if (templateText.find(kPlaceholder) == std::string_view::npos)
        return false;
    return pb::text::startsWith(pb::text::toLower(templateText), "https://");
}

std::string buildSearchUrl(std::string_view templateText, std::string_view query)
{
    if (!isValidSearchTemplate(templateText))
        return {};

    const std::string encoded = pb::text::percentEncode(query);
    std::string url(templateText);
    for (std::size_t at = url.find(kPlaceholder); at != std::string::npos;
         at = url.find(kPlaceholder, at + encoded.size())) {
        url.replace(at, kPlaceholder.size(), encoded);
    }
    return url;
}

} // namespace pb::settings
