#include "check.h"
#include "utils/text.h"

using namespace pb::text;
using pbtest::checkEqual;
using pbtest::checkTrue;

int main()
{
    checkEqual(toLower("ExAmPle.COM"), "example.com", "toLower");
    checkEqual(std::string(trim("  \t value \r\n")), "value", "trim");
    checkEqual(std::string(trim("   ")), "", "trim all space");

    const auto parts = split("a.b.c", '.');
    checkEqual(static_cast<long long>(parts.size()), 3, "split size");
    checkEqual(std::string(parts[0]), "a", "split first");
    checkEqual(std::string(parts[2]), "c", "split last");
    checkEqual(static_cast<long long>(split("", '.').size()), 0, "split empty");
    checkEqual(static_cast<long long>(split("a.", '.').size()), 2, "split trailing separator");

    checkTrue(startsWith("https://a", "https"), "startsWith");
    checkTrue(!startsWith("ht", "https"), "startsWith shorter than prefix");
    checkTrue(endsWith("example.com", ".com"), "endsWith");

    checkTrue(isSubdomainOf("example.com", "example.com"), "same host is a subdomain of itself");
    checkTrue(isSubdomainOf("cdn.example.com", "example.com"), "real subdomain");
    checkTrue(!isSubdomainOf("notexample.com", "example.com"), "suffix without a dot boundary");
    checkTrue(!isSubdomainOf("example.com", "cdn.example.com"), "parent is not a subdomain");

    return pbtest::finish();
}
