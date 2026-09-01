// A slider, used for text scaling in the appearance settings.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string label: ""
    property real from: 0.0
    property real to: 1.0
    property real value: 0.0
    property real stepSize: 0.0
    property string valueText: ""

    signal moved(real value)

    implicitHeight: labelRow.implicitHeight + track.height + Spacing.small
    implicitWidth: 220

    Row {
        id: labelRow
        width: parent.width
        visible: root.label !== "" || root.valueText !== ""
        spacing: Spacing.small

        Text {
            width: parent.width - readout.width - Spacing.small
            text: root.label
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.body
            elide: Text.ElideRight
        }

        Text {
            id: readout
            text: root.valueText
            color: Colors.textSubtle
            font.family: Typography.monoFamily
            font.pixelSize: Typography.label
        }
    }

    Slider {
        id: track

        anchors.top: labelRow.visible ? labelRow.bottom : parent.top
        anchors.topMargin: labelRow.visible ? Spacing.small : 0
        width: parent.width
        height: 24

        from: root.from
        to: root.to
        value: root.value
        stepSize: root.stepSize
        onMoved: root.moved(value)

        Accessible.role: Accessible.Slider
        Accessible.name: root.label

        background: Rectangle {
            x: 0
            y: track.height / 2 - height / 2
            width: track.availableWidth
            height: 4
            radius: 2
            color: Colors.glassFill
            border.width: 1
            border.color: Colors.glassBorder

            Rectangle {
                width: track.visualPosition * parent.width
                height: parent.height
                radius: parent.radius
                color: Colors.accent
            }
        }

        handle: Rectangle {
            x: track.leftPadding + track.visualPosition * (track.availableWidth - width)
            y: track.height / 2 - height / 2
            width: 18
            height: 18
            radius: 9
            color: Colors.text
            border.width: 1
            border.color: Colors.glassBorderStrong
            antialiasing: true
            scale: track.pressed && !Theme.reducedMotion ? 1.15 : 1.0

            Behavior on scale {
                NumberAnimation {
                    duration: Theme.instant
                }
            }
        }
    }
}
