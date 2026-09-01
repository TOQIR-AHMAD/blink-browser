// What a site is allowed to do, and what the browser remembers about it.
//
// PLAN.md §23: every powerful capability defaults to "ask", nothing is ever
// granted silently, and an unrecognised capability is refused rather than
// waved through. Decisions live in memory for the session only - there is no
// permission database, so a site cannot be granted the camera "forever".

#ifndef PB_PERMISSIONS_CORE_PERMISSION_POLICY_H
#define PB_PERMISSIONS_CORE_PERMISSION_POLICY_H

#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pb::permissions {

enum class Feature {
    Camera,
    Microphone,
    CameraAndMicrophone,
    Location,
    Notifications,
    ClipboardRead,
    Midi,
    Usb,
    Bluetooth,
    ScreenCapture,
    WindowCapture,
    FileAccess,
    Serial,
    Hid,
    Unknown,
};

enum class Decision {
    Ask,
    Allow,
    Deny,
};

struct Grant {
    std::string origin;
    Feature feature = Feature::Unknown;
    Decision decision = Decision::Ask;
};

// Human-readable, used by the prompt and the dashboard. Stable strings: the UI
// looks them up rather than switching on the enum in QML.
std::string_view featureName(Feature feature);
std::string_view featureDescription(Feature feature);

class PermissionPolicy
{
public:
    // Ask for everything the browser supports, Deny for anything it does not
    // recognise: a capability nobody reviewed is not one to grant.
    static Decision defaultDecision(Feature feature);

    Decision decisionFor(std::string_view origin, Feature feature) const;

    // Records the user's answer for this session. Passing Decision::Ask
    // forgets a previous answer.
    void remember(std::string_view origin, Feature feature, Decision decision);

    void forget(std::string_view origin, Feature feature);
    void forgetOrigin(std::string_view origin);
    void clear();

    std::vector<Grant> grants() const;
    std::size_t grantedCount() const;
    std::size_t deniedCount() const;

private:
    std::map<std::pair<std::string, Feature>, Decision> m_decisions;
};

} // namespace pb::permissions

#endif // PB_PERMISSIONS_CORE_PERMISSION_POLICY_H
