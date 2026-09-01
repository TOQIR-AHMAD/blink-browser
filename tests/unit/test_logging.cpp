// Verifies that URL redaction never lets a path, query, fragment or credential
// reach a log line (PLAN.md section 31).

#include "utils/logging.h"

#include <cstdio>
#include <string>

namespace {

int failures = 0;

void expectEqual(const std::string &actual, const std::string &expected, const char *what)
{
    if (actual != expected) {
        std::fprintf(stderr, "FAIL %s: expected \"%s\", got \"%s\"\n", what, expected.c_str(),
                     actual.c_str());
        ++failures;
    }
}

using pb::log::redactUrl;
using pb::log::UrlDetail;

void schemeOnlyKeepsNothingButTheScheme()
{
    const auto detail = UrlDetail::SchemeOnly;
    expectEqual(redactUrl("https://example.com/search?q=secret#frag", detail),
                "https://<redacted>", "scheme-only full URL");
    expectEqual(redactUrl("http://example.com", detail), "http://<redacted>",
                "scheme-only bare host");
    expectEqual(redactUrl("https://user:pw@example.com/", detail), "https://<redacted>",
                "scheme-only credentials");
}

void hostDetailKeepsHostOnly()
{
    const auto detail = UrlDetail::SchemeAndHost;
    expectEqual(redactUrl("https://example.com/search?q=secret#frag", detail),
                "https://example.com/<redacted>", "host detail full URL");
    expectEqual(redactUrl("https://example.com/", detail), "https://example.com",
                "host detail trailing slash");
    expectEqual(redactUrl("https://example.com:8443/a", detail),
                "https://example.com:8443/<redacted>", "host detail port");
    expectEqual(redactUrl("https://user:pw@example.com/a", detail),
                "https://example.com/<redacted>", "host detail strips credentials");
}

void opaqueAndMalformedInputIsRedacted()
{
    for (const auto detail : {UrlDetail::SchemeOnly, UrlDetail::SchemeAndHost}) {
        expectEqual(redactUrl("about:blank", detail), "about:<redacted>", "about URL");
        expectEqual(redactUrl("data:text/html,<h1>hi</h1>", detail), "data:<redacted>",
                    "data URL");
        expectEqual(redactUrl("javascript:steal(document.cookie)", detail),
                    "javascript:<redacted>", "javascript URL");
        expectEqual(redactUrl("example.com/path", detail), "<redacted>", "no scheme");
        expectEqual(redactUrl("", detail), "<redacted>", "empty input");
        expectEqual(redactUrl("https://", detail), "https://<redacted>", "empty authority");
    }
}

} // namespace

int main()
{
    schemeOnlyKeepsNothingButTheScheme();
    hostDetailKeepsHostOnly();
    opaqueAndMalformedInputIsRedacted();

    if (failures > 0) {
        std::fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    std::fprintf(stderr, "all checks passed\n");
    return 0;
}
