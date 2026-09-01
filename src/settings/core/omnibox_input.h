// Decides whether what the user typed is an address or a search.
//
// This is where "type a query, get a search" happens, so it is also where a
// mistake would leak: treating an address as a search sends a URL the user
// meant to visit to the search provider. The rules below therefore prefer
// navigation whenever the input plausibly names a host, and refuse to turn
// script URLs into navigations at all.

#ifndef PB_SETTINGS_CORE_OMNIBOX_INPUT_H
#define PB_SETTINGS_CORE_OMNIBOX_INPUT_H

#include <string>
#include <string_view>

namespace pb::settings {

enum class OmniboxAction {
    Nothing, // empty input
    Navigate,
    Search,
};

struct OmniboxResult {
    OmniboxAction action = OmniboxAction::Nothing;
    // For Navigate: the absolute URL to load.
    // For Search: the raw search terms, not yet placed in a provider template.
    std::string value;
};

OmniboxResult classifyInput(std::string_view input);

// True for schemes the address bar must never navigate to directly, because
// typing or pasting one would run script in the context of the current page.
bool isDangerousScheme(std::string_view scheme);

} // namespace pb::settings

#endif // PB_SETTINGS_CORE_OMNIBOX_INPUT_H
