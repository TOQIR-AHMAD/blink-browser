// The permission request panel.
//
// It slides down from under the address bar, names the site and the exact
// capability, and offers Block first. There is no "remember forever": the
// remembered answer lasts until the browser closes, and the panel says so
// (PLAN.md §23).

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string origin: ""
    property int feature: -1
    property string featureName: ""
    property string featureDescription: ""
    property bool remember: false
    readonly property bool active: root.feature >= 0

    signal allowed(bool remember)
    signal blocked(bool remember)

    visible: active
    implicitHeight: card.implicitHeight

    function ask(origin, feature, name, description) {
        root.origin = origin;
        root.feature = feature;
        root.featureName = name;
        root.featureDescription = description;
        root.remember = false;
    }

    function dismiss() {
        root.feature = -1;
        root.origin = "";
    }

    GlassCard {
        id: card

        width: parent.width
        title: qsTr("%1 wants to %2").arg(root.origin).arg(root.featureDescription)
        subtitle: qsTr("Nothing happens unless you allow it.")

        GlassToggle {
            width: parent.width
            checked: root.remember
            label: qsTr("Remember for this site")
            description: qsTr("Until the browser closes. Nothing about this choice is saved to disk.")
            onToggled: root.remember = checked
        }

        Row {
            spacing: Spacing.small
            layoutDirection: Qt.RightToLeft
            width: parent.width

            GlassButton {
                text: qsTr("Block")
                onClicked: {
                    root.blocked(root.remember);
                    root.dismiss();
                }
            }

            GlassButton {
                text: qsTr("Allow")
                accented: true
                onClicked: {
                    root.allowed(root.remember);
                    root.dismiss();
                }
            }
        }
    }

    // Slide in.
    y: active ? 0 : -8
    opacity: active ? 1 : 0
    Behavior on y {
        NumberAnimation {
            duration: Theme.quick
            easing.type: Theme.easing
        }
    }
    Behavior on opacity {
        NumberAnimation {
            duration: Theme.quick
        }
    }
}
