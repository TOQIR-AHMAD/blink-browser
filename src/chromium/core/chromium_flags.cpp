#include "chromium/core/chromium_flags.h"

#include "utils/text.h"

#include <array>

namespace pb::chromium {
namespace {

// Anything that turns off process sandboxing, site isolation or certificate
// validation. PLAN.md §44 forbids all of these outright.
constexpr std::array<std::string_view, 10> kForbiddenPrefixes = {
    "--no-sandbox",
    "--disable-setuid-sandbox",
    "--disable-gpu-sandbox",
    "--disable-seccomp-filter-sandbox",
    "--disable-web-security",
    "--disable-site-isolation-trials",
    "--disable-features=IsolateOrigins",
    "--ignore-certificate-errors",
    "--allow-running-insecure-content",
    "--disable-webrtc-encryption",
};

void append(std::vector<std::string> &out, std::string value)
{
    out.push_back(std::move(value));
}

} // namespace

bool isForbiddenSwitch(const std::string &switchText)
{
    const std::string lowered = pb::text::toLower(switchText);
    for (const auto prefix : kForbiddenPrefixes) {
        if (pb::text::startsWith(lowered, prefix))
            return true;
    }
    // A user switch must not smuggle a forbidden one in through a value, e.g.
    // --enable-features=...,DisableWebSecurity.
    return lowered.find("site-per-process=false") != std::string::npos;
}

std::vector<std::string> buildSwitchList(const FlagsConfig &config)
{
    std::vector<std::string> flags;

    if (config.disableBackgroundNetworking)
        append(flags, "--disable-background-networking");
    if (config.disableComponentUpdater)
        append(flags, "--disable-component-update");
    if (config.disableDomainReliability)
        append(flags, "--disable-domain-reliability");
    if (config.disableHyperlinkAuditing)
        append(flags, "--no-pings");
    if (config.disableCrashReporter)
        append(flags, "--disable-breakpad");
    if (config.disableMediaRouter)
        append(flags, "--media-router=0");

    if (config.webRtcPolicy == WebRtcPolicy::PublicInterfaceOnly)
        append(flags, "--force-webrtc-ip-handling-policy=default_public_interface");

    if (config.dnsMode == DnsMode::SecureDoh && !config.dohTemplate.empty()
        && pb::text::startsWith(pb::text::toLower(config.dohTemplate), "https://")) {
        append(flags, "--dns-over-https-mode=secure");
        append(flags, "--dns-over-https-templates=" + config.dohTemplate);
    }

    // Features whose only purpose is to talk to a service the user did not ask
    // for. Chromium ignores names it does not know, so an entry that a future
    // Chromium drops is harmless.
    append(flags,
           "--disable-features=OptimizationHints,OptimizationHintsFetching,Translate,"
           "AutofillServerCommunication,SafetyTips,MediaRouter,InterestCohortAPI,"
           "PrivacySandboxAdsAPIs,AttributionReporting,TopicsAPI");

    for (const auto &extra : config.extraSwitches) {
        if (!extra.empty() && !isForbiddenSwitch(extra))
            append(flags, extra);
    }

    return flags;
}

std::string buildFlags(const FlagsConfig &config)
{
    std::string out;
    for (const auto &flag : buildSwitchList(config)) {
        if (!out.empty())
            out.push_back(' ');
        out += flag;
    }
    return out;
}

} // namespace pb::chromium
