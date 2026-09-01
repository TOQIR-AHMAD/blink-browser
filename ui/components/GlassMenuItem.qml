// A row in a GlassMenu.
//
// Menu's `delegate` property only applies to items a menu creates from a
// model; MenuItems written out by hand keep whatever the active Qt Quick
// Controls style gives them, which on this dark chrome meant near-invisible
// text and a white highlight bar. So the styling lives here, on the item.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

MenuItem {
    id: control

    // Shown right-aligned, dimmed: "Ctrl+T" and friends.
    property string shortcutText: ""
    property bool danger: false

    implicitWidth: Math.max(240, label.implicitWidth + shortcutLabel.implicitWidth
                                 + Spacing.large * 3)
    implicitHeight: 34
    padding: 0

    Accessible.name: control.text

    background: Rectangle {
        anchors.fill: parent
        anchors.leftMargin: Spacing.tiny
        anchors.rightMargin: Spacing.tiny
        radius: Radius.small
        color: control.highlighted || control.hovered ? Colors.hover : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: Theme.instant
            }
        }
    }

    contentItem: Item {
        implicitHeight: 34

        Text {
            id: label

            anchors.left: parent.left
            anchors.leftMargin: Spacing.medium
            anchors.right: shortcutLabel.left
            anchors.rightMargin: Spacing.small
            anchors.verticalCenter: parent.verticalCenter
            text: control.text
            color: {
                if (!control.enabled)
                    return Colors.textFaint;
                return control.danger ? Colors.danger : Colors.text;
            }
            font.family: Typography.family
            font.pixelSize: Typography.body
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: shortcutLabel

            anchors.right: parent.right
            anchors.rightMargin: Spacing.medium
            anchors.verticalCenter: parent.verticalCenter
            text: control.shortcutText
            visible: text !== ""
            color: Colors.textFaint
            font.family: Typography.monoFamily
            font.pixelSize: Typography.caption
        }
    }
}
