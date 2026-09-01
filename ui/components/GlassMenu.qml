// A dropdown menu on glass.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Menu {
    id: menu

    implicitWidth: 240
    padding: Spacing.small
    margins: Spacing.small

    background: GlassSurface {
        cornerRadius: Radius.large
        elevated: true
        fillColor: Colors.glassFillStrong
    }

    delegate: MenuItem {
        id: item

        implicitHeight: 34
        implicitWidth: menu.implicitWidth

        background: Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            radius: Radius.small
            color: item.highlighted ? Colors.hover : "transparent"
        }

        contentItem: Row {
            spacing: Spacing.small
            leftPadding: Spacing.small
            rightPadding: Spacing.small

            Text {
                anchors.verticalCenter: parent.verticalCenter
                width: item.width - Spacing.large * 2 - shortcutLabel.width
                text: item.text
                color: item.enabled ? Colors.text : Colors.textFaint
                font.family: Typography.family
                font.pixelSize: Typography.body
                elide: Text.ElideRight
            }

            Text {
                id: shortcutLabel
                anchors.verticalCenter: parent.verticalCenter
                text: item.action && item.action.shortcut ? item.action.shortcut : ""
                color: Colors.textFaint
                font.family: Typography.monoFamily
                font.pixelSize: Typography.caption
            }
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: Theme.instant
        }
        NumberAnimation {
            property: "scale"
            from: 0.96
            to: 1.0
            duration: Theme.quick
            easing.type: Theme.easing
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            to: 0.0
            duration: Theme.instant
        }
    }
}
