// The switch list is the browser's contract with Chromium. These tests are the
// guard rail for PLAN.md §44: the sandbox and certificate validation can never
// be turned off, not even through user-supplied switches.

#include "check.h"
#include "chromium/core/chromium_flags.h"

#include <algorithm>

using namespace pb::chromium;
using pbtest::checkTrue;

namespace {

bool contains(const std::string &haystack, const std::string &needle)
{
    return haystack.find(needle) != std::string::npos;
}

void defaultsArePrivate()
{
    const std::string flags = buildFlags(FlagsConfig{});
    checkTrue(contains(flags, "--disable-background-networking"), "background networking off");
    checkTrue(contains(flags, "--disable-component-update"), "component updater off");
    checkTrue(contains(flags, "--disable-domain-reliability"), "domain reliability off");
    checkTrue(contains(flags, "--no-pings"), "hyperlink auditing off");
    checkTrue(contains(flags, "--disable-breakpad"), "crash reporter off");
    checkTrue(contains(flags, "default_public_interface"), "webrtc limited to public interface");
    checkTrue(contains(flags, "OptimizationHints"), "optimization hints disabled");
    checkTrue(contains(flags, "AutofillServerCommunication"), "autofill server calls disabled");
}

void sandboxCanNeverBeDisabled()
{
    FlagsConfig config;
    config.extraSwitches = {
        "--no-sandbox",
        "--disable-web-security",
        "--ignore-certificate-errors",
        "--disable-gpu-sandbox",
        "--site-per-process=false",
        "--DISABLE-SETUID-SANDBOX",
        "--enable-logging", // a harmless one, which must survive
    };
    const std::string flags = buildFlags(config);

    checkTrue(!contains(flags, "no-sandbox"), "--no-sandbox stripped");
    checkTrue(!contains(flags, "disable-web-security"), "--disable-web-security stripped");
    checkTrue(!contains(flags, "ignore-certificate-errors"), "cert bypass stripped");
    checkTrue(!contains(flags, "disable-gpu-sandbox"), "gpu sandbox switch stripped");
    checkTrue(!contains(flags, "site-per-process=false"), "site isolation bypass stripped");
    checkTrue(!contains(flags, "SETUID"), "case-insensitive match");
    checkTrue(contains(flags, "--enable-logging"), "unrelated user switch kept");

    checkTrue(isForbiddenSwitch("--no-sandbox"), "isForbiddenSwitch reports the sandbox switch");
    checkTrue(!isForbiddenSwitch("--enable-logging"), "isForbiddenSwitch allows normal switches");
}

void secureDnsOnlyWhenConfigured()
{
    FlagsConfig system;
    checkTrue(!contains(buildFlags(system), "dns-over-https"), "system DNS adds no DoH switch");

    FlagsConfig doh;
    doh.dnsMode = DnsMode::SecureDoh;
    doh.dohTemplate = "https://dns.example/dns-query{?dns}";
    const std::string flags = buildFlags(doh);
    checkTrue(contains(flags, "--dns-over-https-mode=secure"), "DoH mode set");
    checkTrue(contains(flags, "--dns-over-https-templates=https://dns.example/dns-query{?dns}"),
              "DoH template passed through");

    FlagsConfig insecureTemplate;
    insecureTemplate.dnsMode = DnsMode::SecureDoh;
    insecureTemplate.dohTemplate = "http://dns.example/dns-query";
    checkTrue(!contains(buildFlags(insecureTemplate), "dns-over-https"),
              "a plain-http DoH template is refused");

    FlagsConfig emptyTemplate;
    emptyTemplate.dnsMode = DnsMode::SecureDoh;
    checkTrue(!contains(buildFlags(emptyTemplate), "dns-over-https"),
              "an empty DoH template is refused");
}

void switchesAreSpaceSeparated()
{
    const auto list = buildSwitchList(FlagsConfig{});
    checkTrue(!list.empty(), "switch list is not empty");
    checkTrue(std::none_of(list.begin(), list.end(),
                           [](const std::string &s) { return s.find(' ') != std::string::npos; }),
              "no switch contains a space");
}

} // namespace

int main()
{
    defaultsArePrivate();
    sandboxCanNeverBeDisabled();
    secureDnsOnlyWhenConfigured();
    switchesAreSpaceSeparated();
    return pbtest::finish();
}
