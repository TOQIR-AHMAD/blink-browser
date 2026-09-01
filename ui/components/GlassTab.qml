// One tab in the strip.
//
// Shows the favicon (or a loading ring, or a crash mark), the title, an audio
// indicator, and a close button that appears on hover or when the tab is the
// active one.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

AbstractButton {
    id: control

    // Assigned by the tab strip from the model role.
    property var tabData: null
    property bool active: false

    signal closeRequested()

    implicitHeight: 34
    hoverEnabled: true
    focusPolicy: Qt.TabFocus

    Accessible.role: Accessible.PageTab
    Accessible.name: control.tabData ? control.tabData.displayTitle : ""
    Accessible.selected: control.active

    background: GlassSurface {
        cornerRadius: Radius.medium
        interactive: true
        hovered: control.hovered
        down: control.down
        selected: control.active
        fillColor: control.active ? Colors.glassFillStrong : Qt.rgba(0, 0, 0, 0)
        borderWidth: control.active ? 1 : 0
        showHighlight: control.active
    }

    contentItem: Item {
        implicitWidth: 160

        Item {
            id: indicator

            anchors.left: parent.left
            anchors.leftMargin: Spacing.small
            anchors.verticalCenter: parent.verticalCenter
            width: 16
            height: 16

            Image {
                id: favicon
                anchors.fill: parent
                source: control.tabData && !control.tabData.loading && !control.tabData.crashed
                        ? control.tabData.iconUrl : ""
                visible: source !== "" && status === Image.Ready
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 32
                sourceSize.height: 32
                asynchronous: true
                // Favicons come from the site itself and are subject to the
                // same blocking rules as any other request.
            }

            // Loading ring.
            Rectangle {
                id: spinner
                anchors.centerIn: parent
                width: 12
                height: 12
                radius: 6
                color: "transparent"
                border.width: 2
                border.color: Colors.accent
                opacity: 0.85
                visible: control.tabData && control.tabData.loading

                RotationAnimation on rotation {
                    running: spinner.visible && !Theme.reducedMotion
                    loops: Animation.Infinite
                    from: 0
                    to: 360
                    duration: 900
                }

                Rectangle {
                    width: 5
                    height: 5
                    radius: 2.5
                    color: Colors.background
                    anchors.right: parent.right
                    anchors.top: parent.top
                }
            }

            Text {
                anchors.centerIn: parent
                text: "⚠"
                color: Colors.danger
                font.pixelSize: 13
                visible: control.tabData && control.tabData.crashed
            }

            // Fallback dot when a site has no favicon yet.
            Rectangle {
                anchors.centerIn: parent
                width: 6
                height: 6
                radius: 3
                color: Colors.textFaint
                visible: !favicon.visible && !spinner.visible
                         && !(control.tabData && control.tabData.crashed)
            }
        }

        Text {
            anchors.left: indicator.right
            anchors.leftMargin: Spacing.small
            anchors.right: audio.visible ? audio.left : closeButton.left
            anchors.rightMargin: Spacing.tiny
            anchors.verticalCenter: parent.verticalCenter
            text: control.tabData ? control.tabData.displayTitle : ""
            color: control.active ? Colors.text : Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.label
            elide: Text.ElideRight
        }

        GlassButton {
            id: audio

            anchors.right: closeButton.left
            anchors.verticalCenter: parent.verticalCenter
            visible: control.tabData && (control.tabData.audible || control.tabData.muted)
            width: 22
            height: 22
            flat: true
            glyph: control.tabData && control.tabData.muted ? "🔇" : "🔊"
            tooltip: control.tabData && control.tabData.muted ? qsTr("Unmute tab") : qsTr("Mute tab")
            onClicked: if (control.tabData) control.tabData.toggleMuted()
        }

        GlassButton {
            id: closeButton

            anchors.right: parent.right
            anchors.rightMargin: Spacing.tiny
            anchors.verticalCenter: parent.verticalCenter
            width: 22
            height: 22
            flat: true
            glyph: "✕"
            tooltip: qsTr("Close tab")
            opacity: control.active || control.hovered ? 1.0 : 0.0
            visible: opacity > 0.01
            onClicked: control.closeRequested()

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.instant
                }
            }
        }
    }
}
