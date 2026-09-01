#include "permissions/core/permission_policy.h"

#include <algorithm>

namespace pb::permissions {

std::string_view featureName(Feature feature)
{
    switch (feature) {
    case Feature::Camera:
        return "Camera";
    case Feature::Microphone:
        return "Microphone";
    case Feature::CameraAndMicrophone:
        return "Camera and microphone";
    case Feature::Location:
        return "Location";
    case Feature::Notifications:
        return "Notifications";
    case Feature::ClipboardRead:
        return "Clipboard";
    case Feature::Midi:
        return "MIDI devices";
    case Feature::Usb:
        return "USB devices";
    case Feature::Bluetooth:
        return "Bluetooth";
    case Feature::ScreenCapture:
        return "Screen sharing";
    case Feature::WindowCapture:
        return "Window sharing";
    case Feature::FileAccess:
        return "File access";
    case Feature::Serial:
        return "Serial devices";
    case Feature::Hid:
        return "Input devices";
    case Feature::Unknown:
        break;
    }
    return "Unrecognised capability";
}

std::string_view featureDescription(Feature feature)
{
    switch (feature) {
    case Feature::Camera:
        return "see through your camera";
    case Feature::Microphone:
        return "hear your microphone";
    case Feature::CameraAndMicrophone:
        return "see and hear you";
    case Feature::Location:
        return "know where you are";
    case Feature::Notifications:
        return "send you notifications";
    case Feature::ClipboardRead:
        return "read what you copied";
    case Feature::Midi:
        return "use your MIDI devices";
    case Feature::Usb:
        return "talk to a USB device";
    case Feature::Bluetooth:
        return "talk to a Bluetooth device";
    case Feature::ScreenCapture:
        return "watch your screen";
    case Feature::WindowCapture:
        return "watch a window on your screen";
    case Feature::FileAccess:
        return "read files you choose";
    case Feature::Serial:
        return "talk to a serial device";
    case Feature::Hid:
        return "talk to an input device";
    case Feature::Unknown:
        break;
    }
    return "use a capability this browser does not recognise";
}

Decision PermissionPolicy::defaultDecision(Feature feature)
{
    return feature == Feature::Unknown ? Decision::Deny : Decision::Ask;
}

Decision PermissionPolicy::decisionFor(std::string_view origin, Feature feature) const
{
    if (origin.empty())
        return Decision::Deny; // an opaque origin has nobody to grant to
    const auto it = m_decisions.find({ std::string(origin), feature });
    if (it != m_decisions.end())
        return it->second;
    return defaultDecision(feature);
}

void PermissionPolicy::remember(std::string_view origin, Feature feature, Decision decision)
{
    if (origin.empty() || feature == Feature::Unknown)
        return;
    if (decision == Decision::Ask) {
        forget(origin, feature);
        return;
    }
    m_decisions[{ std::string(origin), feature }] = decision;
}

void PermissionPolicy::forget(std::string_view origin, Feature feature)
{
    m_decisions.erase({ std::string(origin), feature });
}

void PermissionPolicy::forgetOrigin(std::string_view origin)
{
    for (auto it = m_decisions.begin(); it != m_decisions.end();) {
        if (it->first.first == origin)
            it = m_decisions.erase(it);
        else
            ++it;
    }
}

void PermissionPolicy::clear()
{
    m_decisions.clear();
}

std::vector<Grant> PermissionPolicy::grants() const
{
    std::vector<Grant> result;
    result.reserve(m_decisions.size());
    for (const auto &entry : m_decisions)
        result.push_back({ entry.first.first, entry.first.second, entry.second });
    return result;
}

std::size_t PermissionPolicy::grantedCount() const
{
    return static_cast<std::size_t>(
        std::count_if(m_decisions.begin(), m_decisions.end(),
                      [](const auto &entry) { return entry.second == Decision::Allow; }));
}

std::size_t PermissionPolicy::deniedCount() const
{
    return static_cast<std::size_t>(
        std::count_if(m_decisions.begin(), m_decisions.end(),
                      [](const auto &entry) { return entry.second == Decision::Deny; }));
}

} // namespace pb::permissions
