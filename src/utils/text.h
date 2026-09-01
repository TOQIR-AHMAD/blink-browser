// Small string helpers shared by the Qt-free core. Deliberately minimal: these
// exist so the core does not need QString (PLAN.md §45, keep dependencies out of
// code that must be testable without Qt).

#ifndef PB_UTILS_TEXT_H
#define PB_UTILS_TEXT_H

#include <string>
#include <string_view>
#include <vector>

namespace pb::text {

// ASCII-only case folding. Hostnames and filter rules are ASCII by the time
// they reach the core; percent/punycode decoding happens upstream in Qt.
std::string toLower(std::string_view value);

std::string_view trim(std::string_view value);

std::vector<std::string_view> split(std::string_view value, char separator);

bool startsWith(std::string_view value, std::string_view prefix);
bool endsWith(std::string_view value, std::string_view suffix);

// True when `value` equals `suffix` or ends with "." + suffix.
bool isSubdomainOf(std::string_view value, std::string_view suffix);

// Percent-encodes everything outside the RFC 3986 unreserved set, so a search
// term cannot break out of the query component of a URL.
std::string percentEncode(std::string_view value);

} // namespace pb::text

#endif // PB_UTILS_TEXT_H
