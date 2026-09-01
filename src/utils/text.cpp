#include "utils/text.h"

namespace pb::text {

std::string toLower(std::string_view value)
{
    std::string out;
    out.reserve(value.size());
    for (const char c : value)
        out.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c);
    return out;
}

std::string_view trim(std::string_view value)
{
    const auto isSpace = [](char c) {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
    };
    while (!value.empty() && isSpace(value.front()))
        value.remove_prefix(1);
    while (!value.empty() && isSpace(value.back()))
        value.remove_suffix(1);
    return value;
}

std::vector<std::string_view> split(std::string_view value, char separator)
{
    std::vector<std::string_view> parts;
    if (value.empty())
        return parts;

    std::size_t start = 0;
    while (true) {
        const std::size_t next = value.find(separator, start);
        if (next == std::string_view::npos) {
            parts.push_back(value.substr(start));
            break;
        }
        parts.push_back(value.substr(start, next - start));
        start = next + 1;
    }
    return parts;
}

bool startsWith(std::string_view value, std::string_view prefix)
{
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(std::string_view value, std::string_view suffix)
{
    return value.size() >= suffix.size()
        && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string percentEncode(std::string_view value)
{
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::string out;
    out.reserve(value.size());
    for (const unsigned char c : value) {
        const bool unreserved = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~';
        if (unreserved) {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(kHex[c >> 4]);
            out.push_back(kHex[c & 0x0F]);
        }
    }
    return out;
}

bool isSubdomainOf(std::string_view value, std::string_view suffix)
{
    if (suffix.empty() || value.empty())
        return false;
    if (value == suffix)
        return true;
    return value.size() > suffix.size() + 1 && endsWith(value, suffix)
        && value[value.size() - suffix.size() - 1] == '.';
}

} // namespace pb::text
