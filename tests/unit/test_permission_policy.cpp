#include "check.h"
#include "permissions/core/permission_policy.h"

using namespace pb::permissions;
using pbtest::checkEqual;
using pbtest::checkTrue;

int main()
{
    // PLAN.md §23: everything asks, nothing is granted silently.
    const Feature everyFeature[] = {
        Feature::Camera,     Feature::Microphone,    Feature::CameraAndMicrophone,
        Feature::Location,   Feature::Notifications, Feature::ClipboardRead,
        Feature::Midi,       Feature::Usb,           Feature::Bluetooth,
        Feature::ScreenCapture, Feature::WindowCapture, Feature::FileAccess,
        Feature::Serial,     Feature::Hid,
    };
    for (const Feature feature : everyFeature) {
        checkTrue(PermissionPolicy::defaultDecision(feature) == Decision::Ask,
                  "every supported capability defaults to ask");
        checkTrue(!featureName(feature).empty(), "every capability has a name for the prompt");
        checkTrue(!featureDescription(feature).empty(), "every capability has a description");
    }
    checkTrue(PermissionPolicy::defaultDecision(Feature::Unknown) == Decision::Deny,
              "an unrecognised capability is refused, not asked about");

    PermissionPolicy policy;
    checkTrue(policy.decisionFor("https://example.com", Feature::Camera) == Decision::Ask,
              "a fresh policy asks");
    checkTrue(policy.decisionFor("", Feature::Camera) == Decision::Deny,
              "an opaque origin is refused");

    policy.remember("https://example.com", Feature::Camera, Decision::Allow);
    checkTrue(policy.decisionFor("https://example.com", Feature::Camera) == Decision::Allow,
              "the answer is remembered for the session");
    checkTrue(policy.decisionFor("https://example.com", Feature::Microphone) == Decision::Ask,
              "one capability does not imply another");
    checkTrue(policy.decisionFor("https://other.example", Feature::Camera) == Decision::Ask,
              "one origin does not imply another");
    checkEqual(static_cast<long long>(policy.grantedCount()), 1, "granted count");

    policy.remember("https://example.com", Feature::Location, Decision::Deny);
    checkEqual(static_cast<long long>(policy.deniedCount()), 1, "denied count");
    checkEqual(static_cast<long long>(policy.grants().size()), 2, "both answers listed");

    policy.remember("https://example.com", Feature::Camera, Decision::Ask);
    checkTrue(policy.decisionFor("https://example.com", Feature::Camera) == Decision::Ask,
              "answering 'ask' forgets the previous answer");

    policy.remember("https://example.com", Feature::Unknown, Decision::Allow);
    checkTrue(policy.decisionFor("https://example.com", Feature::Unknown) == Decision::Deny,
              "an unrecognised capability cannot be granted at all");

    policy.forgetOrigin("https://example.com");
    checkEqual(static_cast<long long>(policy.grants().size()), 0, "forgetting an origin");

    policy.remember("https://a.example", Feature::Camera, Decision::Allow);
    policy.clear();
    checkEqual(static_cast<long long>(policy.grants().size()), 0,
               "clear leaves nothing behind at session end");

    return pbtest::finish();
}
