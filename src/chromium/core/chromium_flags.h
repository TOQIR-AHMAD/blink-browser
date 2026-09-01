// Builds the command line handed to Chromium through QTWEBENGINE_CHROMIUM_FLAGS.
//
// Kept Qt-free so the exact switch list is unit-testable: the tests are what
// guarantee the sandbox is never disabled and that no background network
// feature is silently re-enabled (PLAN.md §44).

#ifndef PB_CHROMIUM_CORE_CHROMIUM_FLAGS_H
#define PB_CHROMIUM_CORE_CHROMIUM_FLAGS_H

#include <string>
#include <vector>

namespace pb::chromium {

enum class DnsMode {
    System,   // resolve through the operating system resolver
    SecureDoh // resolve through the configured DNS-over-HTTPS template
};

enum class WebRtcPolicy {
    // Chromium's default: WebRTC may enumerate local interfaces, which exposes
    // a local IP address to a page that asks.
    Default,
    // Only the public interface is used. Reduces what a page learns; can affect
    // peer-to-peer connectivity on some networks.
    PublicInterfaceOnly,
};

struct FlagsConfig {
    // Every one of these defaults to the privacy-preserving value. Turning any
    // of them off is a user-visible setting, never an internal shortcut.
    bool disableBackgroundNetworking = true;
    bool disableComponentUpdater = true;
    bool disableDomainReliability = true;
    bool disableHyperlinkAuditing = true;
    bool disableCrashReporter = true;
    bool disableMediaRouter = true;
    WebRtcPolicy webRtcPolicy = WebRtcPolicy::PublicInterfaceOnly;

    DnsMode dnsMode = DnsMode::System;
    // Only read when dnsMode is SecureDoh. Must be an https:// template.
    std::string dohTemplate;

    // Extra switches from the user (Advanced settings). They are appended
    // verbatim except for the ones on the refusal list below.
    std::vector<std::string> extraSwitches;
};

// Switches that would weaken Chromium's security model. They are stripped from
// extraSwitches and can never be produced by buildFlags() itself (PLAN.md §44).
bool isForbiddenSwitch(const std::string &switchText);

// Returns the space-separated switch list for QTWEBENGINE_CHROMIUM_FLAGS.
std::string buildFlags(const FlagsConfig &config);

// The individual switches, in the order they are emitted. Exposed for the
// network-audit report and for tests.
std::vector<std::string> buildSwitchList(const FlagsConfig &config);

} // namespace pb::chromium

#endif // PB_CHROMIUM_CORE_CHROMIUM_FLAGS_H
