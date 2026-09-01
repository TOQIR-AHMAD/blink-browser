#include "permissions/permission_manager.h"

#include "privacy/core/blocking_stats.h"

#include <QtCore/QVariantMap>
#ifdef PB_WEB_ENGINE
#  include <QtWebEngineCore/QWebEnginePermission>
#endif

namespace pb::permissions {
namespace {

Feature toFeature(int value)
{
    switch (static_cast<PermissionManager::FeatureValue>(value)) {
    case PermissionManager::Camera:
        return Feature::Camera;
    case PermissionManager::Microphone:
        return Feature::Microphone;
    case PermissionManager::CameraAndMicrophone:
        return Feature::CameraAndMicrophone;
    case PermissionManager::Location:
        return Feature::Location;
    case PermissionManager::Notifications:
        return Feature::Notifications;
    case PermissionManager::ClipboardRead:
        return Feature::ClipboardRead;
    case PermissionManager::Midi:
        return Feature::Midi;
    case PermissionManager::Usb:
        return Feature::Usb;
    case PermissionManager::Bluetooth:
        return Feature::Bluetooth;
    case PermissionManager::ScreenCapture:
        return Feature::ScreenCapture;
    case PermissionManager::WindowCapture:
        return Feature::WindowCapture;
    case PermissionManager::FileAccess:
        return Feature::FileAccess;
    case PermissionManager::Serial:
        return Feature::Serial;
    case PermissionManager::Hid:
        return Feature::Hid;
    case PermissionManager::UnknownFeature:
        break;
    }
    return Feature::Unknown;
}

int fromFeature(Feature feature)
{
    switch (feature) {
    case Feature::Camera:
        return PermissionManager::Camera;
    case Feature::Microphone:
        return PermissionManager::Microphone;
    case Feature::CameraAndMicrophone:
        return PermissionManager::CameraAndMicrophone;
    case Feature::Location:
        return PermissionManager::Location;
    case Feature::Notifications:
        return PermissionManager::Notifications;
    case Feature::ClipboardRead:
        return PermissionManager::ClipboardRead;
    case Feature::Midi:
        return PermissionManager::Midi;
    case Feature::Usb:
        return PermissionManager::Usb;
    case Feature::Bluetooth:
        return PermissionManager::Bluetooth;
    case Feature::ScreenCapture:
        return PermissionManager::ScreenCapture;
    case Feature::WindowCapture:
        return PermissionManager::WindowCapture;
    case Feature::FileAccess:
        return PermissionManager::FileAccess;
    case Feature::Serial:
        return PermissionManager::Serial;
    case Feature::Hid:
        return PermissionManager::Hid;
    case Feature::Unknown:
        break;
    }
    return PermissionManager::UnknownFeature;
}

Decision toDecision(int value)
{
    switch (static_cast<PermissionManager::DecisionValue>(value)) {
    case PermissionManager::Allow:
        return Decision::Allow;
    case PermissionManager::Deny:
        return Decision::Deny;
    case PermissionManager::Ask:
        break;
    }
    return Decision::Ask;
}

int fromDecision(Decision decision)
{
    switch (decision) {
    case Decision::Allow:
        return PermissionManager::Allow;
    case Decision::Deny:
        return PermissionManager::Deny;
    case Decision::Ask:
        break;
    }
    return PermissionManager::Ask;
}

} // namespace

PermissionManager::PermissionManager(pb::privacy::BlockingStats *stats, QObject *parent)
    : QObject(parent)
    , m_stats(stats)
{
}

int PermissionManager::featureFromWebEngine(int permissionType) const
{
#ifndef PB_WEB_ENGINE
    // Without a web engine nothing can ask for a capability; anything that
    // claims to is refused.
    Q_UNUSED(permissionType)
    return UnknownFeature;
#else
    using PermissionType = QWebEnginePermission::PermissionType;
    switch (static_cast<PermissionType>(permissionType)) {
    case PermissionType::MediaVideoCapture:
        return Camera;
    case PermissionType::MediaAudioCapture:
        return Microphone;
    case PermissionType::MediaAudioVideoCapture:
        return CameraAndMicrophone;
    case PermissionType::DesktopVideoCapture:
        return ScreenCapture;
    case PermissionType::DesktopAudioVideoCapture:
        return ScreenCapture;
    case PermissionType::Geolocation:
        return Location;
    case PermissionType::Notifications:
        return Notifications;
    case PermissionType::ClipboardReadWrite:
        return ClipboardRead;
    case PermissionType::LocalFontsAccess:
        return FileAccess;
    default:
        break;
    }
    // Includes MouseLock and anything a newer Qt adds: not recognised, so not
    // granted.
    return UnknownFeature;
#endif
}

int PermissionManager::decisionFor(const QString &origin, int feature) const
{
    return fromDecision(m_policy.decisionFor(origin.toStdString(), toFeature(feature)));
}

void PermissionManager::remember(const QString &origin, int feature, int decision)
{
    m_policy.remember(origin.toStdString(), toFeature(feature), toDecision(decision));
    recordOutcome(decision);
    Q_EMIT changed();
}

void PermissionManager::recordOutcome(int decision)
{
    if (!m_stats)
        return;
    if (decision == Allow)
        m_stats->recordPermissionGranted();
    else if (decision == Deny)
        m_stats->recordPermissionDenied();
}

void PermissionManager::forget(const QString &origin, int feature)
{
    m_policy.forget(origin.toStdString(), toFeature(feature));
    Q_EMIT changed();
}

void PermissionManager::clear()
{
    m_policy.clear();
    Q_EMIT changed();
}

QString PermissionManager::featureName(int feature) const
{
    const std::string_view name = pb::permissions::featureName(toFeature(feature));
    return QString::fromUtf8(name.data(), static_cast<int>(name.size()));
}

QString PermissionManager::featureDescription(int feature) const
{
    const std::string_view text = pb::permissions::featureDescription(toFeature(feature));
    return QString::fromUtf8(text.data(), static_cast<int>(text.size()));
}

QVariantList PermissionManager::grants() const
{
    QVariantList list;
    for (const Grant &grant : m_policy.grants()) {
        QVariantMap entry;
        entry[QStringLiteral("origin")] = QString::fromStdString(grant.origin);
        entry[QStringLiteral("feature")] = fromFeature(grant.feature);
        entry[QStringLiteral("featureName")] = featureName(fromFeature(grant.feature));
        entry[QStringLiteral("allowed")] = grant.decision == Decision::Allow;
        list.append(entry);
    }
    return list;
}

int PermissionManager::grantedCount() const
{
    return static_cast<int>(m_policy.grantedCount());
}

int PermissionManager::deniedCount() const
{
    return static_cast<int>(m_policy.deniedCount());
}

} // namespace pb::permissions
