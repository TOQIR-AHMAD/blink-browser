#include "check.h"
#include "settings/core/omnibox_input.h"

using namespace pb::settings;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

void expectNavigate(const char *input, const char *url)
{
    const OmniboxResult result = classifyInput(input);
    checkTrue(result.action == OmniboxAction::Navigate, input);
    checkEqual(result.value, url, input);
}

void expectSearch(const char *input, const char *terms)
{
    const OmniboxResult result = classifyInput(input);
    checkTrue(result.action == OmniboxAction::Search, input);
    checkEqual(result.value, terms, input);
}

} // namespace

int main()
{
    // Explicit schemes are honoured as typed.
    expectNavigate("https://example.com/a?b=c", "https://example.com/a?b=c");
    expectNavigate("http://example.com", "http://example.com");
    expectNavigate("about:blank", "about:blank");
    expectNavigate("  https://example.com  ", "https://example.com");

    // Bare hosts get https, local addresses get http.
    expectNavigate("example.com", "https://example.com");
    expectNavigate("example.com/path?q=1", "https://example.com/path?q=1");
    expectNavigate("sub.example.co.uk", "https://sub.example.co.uk");
    expectNavigate("example.com:8443/x", "https://example.com:8443/x");
    expectNavigate("localhost", "http://localhost");
    expectNavigate("localhost:3000/app", "http://localhost:3000/app");
    expectNavigate("127.0.0.1:8080", "http://127.0.0.1:8080");

    // Anything else is a search, and the terms are passed through untouched.
    expectSearch("privacy browser", "privacy browser");
    expectSearch("what is 1.5 kg in lbs", "what is 1.5 kg in lbs");
    expectSearch("example", "example");
    expectSearch("version 1.2", "version 1.2");
    expectSearch("c++ std::string", "c++ std::string");

    // Script and data URLs must never become navigations: pasting one into the
    // address bar is the classic self-XSS route.
    expectSearch("javascript:alert(document.cookie)", "javascript:alert(document.cookie)");
    expectSearch("JavaScript:void(0)", "JavaScript:void(0)");
    expectSearch("data:text/html,<script>x</script>", "data:text/html,<script>x</script>");
    checkTrue(isDangerousScheme("javascript"), "javascript is dangerous");
    checkTrue(isDangerousScheme("DATA"), "data is dangerous, case-insensitively");
    checkTrue(!isDangerousScheme("https"), "https is not");

    // Local files.
    expectNavigate("C:\\Users\\me\\notes.html", "file:///C:/Users/me/notes.html");
    expectNavigate("/etc/hosts", "file:///etc/hosts");

    const OmniboxResult empty = classifyInput("   ");
    checkTrue(empty.action == OmniboxAction::Nothing, "blank input does nothing");

    return pbtest::finish();
}
