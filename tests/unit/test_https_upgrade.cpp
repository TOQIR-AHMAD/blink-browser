#include "check.h"
#include "network/core/https_upgrade.h"

using namespace pb::net;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

std::string upgrade(const HttpsUpgrade &upgrader, const char *url, bool mainFrame = true)
{
    return upgrader.upgradedUrl(UrlInfo::parse(url), mainFrame);
}

} // namespace

int main()
{
    HttpsUpgrade upgrader;

    checkEqual(upgrade(upgrader, "http://example.com/a?b=c#d"), "https://example.com/a?b=c#d",
               "path, query and fragment are preserved");
    checkEqual(upgrade(upgrader, "http://example.com"), "https://example.com/",
               "a bare host gets a root path");
    checkEqual(upgrade(upgrader, "http://example.com:8080/x"), "https://example.com:8080/x",
               "a non-default port is kept");
    checkEqual(upgrade(upgrader, "http://example.com:80/x"), "https://example.com/x",
               "the default http port is dropped");

    checkEqual(upgrade(upgrader, "https://example.com/"), "", "https is left alone");
    checkEqual(upgrade(upgrader, "http://example.com/", false), "",
               "sub-resources are not rewritten");
    checkEqual(upgrade(upgrader, "about:blank"), "", "internal pages are left alone");

    // Local and private addresses.
    checkTrue(isLocalHost("localhost"), "localhost");
    checkTrue(isLocalHost("127.0.0.1"), "loopback");
    checkTrue(isLocalHost("192.168.1.10"), "private range");
    checkTrue(isLocalHost("10.1.2.3"), "private range 10/8");
    checkTrue(isLocalHost("172.16.0.1"), "private range 172.16/12");
    checkTrue(isLocalHost("router"), "single-label intranet name");
    checkTrue(isLocalHost("printer.local"), "mDNS name");
    checkTrue(!isLocalHost("example.com"), "a public name is not local");
    checkTrue(!isLocalHost("172.32.0.1"), "just outside the private range");
    checkEqual(upgrade(upgrader, "http://localhost:3000/app"), "", "localhost is not upgraded");
    checkEqual(upgrade(upgrader, "http://192.168.1.10/"), "", "private address is not upgraded");

    // A host that has already failed over https is not retried all session.
    checkTrue(!upgrader.isKnownHttpOnly("legacy.test"), "nothing known yet");
    upgrader.recordFailure("Legacy.Test");
    checkTrue(upgrader.isKnownHttpOnly("legacy.test"), "failure remembered case-insensitively");
    checkEqual(upgrade(upgrader, "http://legacy.test/page"), "",
               "a known http-only host is not upgraded again");
    checkEqual(static_cast<long long>(upgrader.knownHttpOnlyCount()), 1, "one host remembered");

    upgrader.clear();
    checkTrue(!upgrader.isKnownHttpOnly("legacy.test"),
              "the http-only list is session state and can be cleared");

    return pbtest::finish();
}
