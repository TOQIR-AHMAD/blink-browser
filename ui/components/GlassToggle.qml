// A switch. Used for every on/off setting, including the ones the browser
// refuses to offer - those are shown by PrivacyRow as fixed text instead, so a
// toggle here always means something the user can actually change.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

AbstractButton {
    id: control

    property string label: ""
    property string description: ""

    checkable: true
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    implicitHeight: Math.max(28, column.implicitHeight)
    implicitWidth: column.implicitWidth + track.width + Spacing.large
    opacity: enabled ? 1.0 : 0.45

    Accessible.role: Accessible.CheckBox
    Accessible.name: label !== "" ? label : text
    Accessible.checked: checked
    Accessible.onToggleAction: control.toggle()

    Column {
        id: column

        anchors.left: parent.left
        anchors.right: track.left
        anchors.rightMargin: Spacing.large
        anchors.verticalCenter: parent.verticalCenter
        spacing: Spacing.hair

        Text {
            width: parent.width
            text: control.label !== "" ? control.label : control.text
            visible: text !== ""
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: control.description
            visible: text !== ""
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }

    Rectangle {
        id: track

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 44
        height: 26
        radius: height / 2
        color: control.checked ? Colors.accent : Colors.glassFill
        border.width: 1
        border.color: control.checked ? Colors.accent : Colors.glassBorder
        antialiasing: true

        Behavior on color {
            ColorAnimation {
                duration: Theme.quick
                easing.type: Theme.easing
            }
        }

        Rectangle {
            id: knob

            width: 20
            height: 20
            radius: height / 2
            anchors.verticalCenter: parent.verticalCenter
            x: control.checked ? parent.width - width - 3 : 3
            color: control.checked ? Colors.textOnAccent : Colors.text
            opacity: control.checked ? 1.0 : 0.75
            antialiasing: true

            Behavior on x {
                NumberAnimation {
                    duration: Theme.quick
                    easing.type: Theme.easing
                }
            }
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: -3
            radius: height / 2
            color: "transparent"
            border.width: 2
            border.color: Colors.focusRing
            visible: control.visualFocus
        }
    }
}
