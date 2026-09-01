// Bridges Qt WebEngine's permission requests to the session permission policy.
//
// The QML view asks this object what to do with a request. The answer is
// "ask the user" unless the user already answered for that origin in this
// session; nothing is ever granted without a person clicking Allow
// (PLAN.md §23).

#ifndef PB_PERMISSIONS_PERMISSION_MANAGER_H
#define PB_PERMISSIONS_PERMISSION_MANAGER_H

#include "permissions/core/permission_policy.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantList>
#include <QtQml/qqmlregistration.h>

namespace pb::privacy {
class BlockingStats;
}

namespace pb::permissions {

class PermissionManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Provided by the application as App.permissions")

    Q_PROPERTY(int grantedCount READ grantedCount NOTIFY changed)
    Q_PROPERTY(int deniedCount READ deniedCount NOTIFY changed)

public:
    // Mirrors pb::permissions::Feature for QML.
    enum FeatureValue {
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
        UnknownFeature,
    };
    Q_ENUM(FeatureValue)

    enum DecisionValue {
        Ask,
        Allow,
        Deny,
    };
    Q_ENUM(DecisionValue)

    explicit PermissionManager(pb::privacy::BlockingStats *stats, QObject *parent = nullptr);

    // Translates a QWebEnginePermission::PermissionType value. Anything this
    // browser does not know maps to UnknownFeature, which is denied.
    Q_INVOKABLE int featureFromWebEngine(int permissionType) const;

    Q_INVOKABLE int decisionFor(const QString &origin, int feature) const;
    Q_INVOKABLE void remember(const QString &origin, int feature, int decision);
    Q_INVOKABLE void recordOutcome(int decision);
    Q_INVOKABLE void forget(const QString &origin, int feature);
    Q_INVOKABLE void clear();

    Q_INVOKABLE QString featureName(int feature) const;
    Q_INVOKABLE QString featureDescription(int feature) const;
    Q_INVOKABLE QVariantList grants() const;

    int grantedCount() const;
    int deniedCount() const;

Q_SIGNALS:
    void changed();

private:
    PermissionPolicy m_policy;
    pb::privacy::BlockingStats *m_stats = nullptr;
};

} // namespace pb::permissions

#endif // PB_PERMISSIONS_PERMISSION_MANAGER_H
