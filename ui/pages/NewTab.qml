// The new-tab page.
//
// A clock, a search field, and what this session has blocked. No feed, no
// sponsored tiles, no server involved: the shortcuts below are simply the
// pages visited in this session, which the browser already holds in memory and
// forgets on exit (PLAN.md §11).

import QtQuick
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    required property var controller
    required property var privacy
    property bool showClock: true

    signal navigationRequested(url target)

    Rectangle {
        anchors.fill: parent
        color: Colors.background

        // The same aurora as the window chrome, so a new tab feels like part
        // of the browser rather than a blank document.
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: Colors.auroraOne }
                GradientStop { position: 0.55; color: Colors.background }
                GradientStop { position: 1.0; color: Colors.auroraThree }
            }
            opacity: 0.55
        }
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: -parent.height * 0.06
        width: Math.min(parent.width - Spacing.huge * 2, 640)
        spacing: Spacing.xlarge

        Column {
            width: parent.width
            spacing: Spacing.tiny
            visible: root.showClock

            Text {
                id: clock
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: Colors.text
                font.family: Typography.family
                font.pixelSize: Math.round(Typography.display * 1.6)
                font.weight: Font.Light
                font.letterSpacing: Typography.tightLetterSpacing
                text: Qt.formatTime(new Date(), Qt.locale().timeFormat(Locale.ShortFormat))

                Timer {
                    interval: 10000
                    running: root.visible && root.showClock
                    repeat: true
                    triggeredOnStart: true
                    onTriggered: clock.text = Qt.formatTime(
                        new Date(), Qt.locale().timeFormat(Locale.ShortFormat))
                }
            }

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: Colors.textSubtle
                font.family: Typography.family
                font.pixelSize: Typography.body
                text: root.controller && root.controller.privateMode
                      ? qsTr("Private window")
                      : qsTr("Nothing from this session is written to disk")
            }
        }

        GlassTextField {
            id: search

            width: parent.width
            height: 46
            placeholderText: qsTr("Search or enter an address")
            onAccepted: function (text) {
                const target = root.controller.resolveInput(text);
                if (target && target.toString() !== "") {
                    search.text = "";
                    root.navigationRequested(target);
                }
            }
        }

        // Session shortcuts.
        Flow {
            id: shortcuts

            width: parent.width
            spacing: Spacing.small
            visible: repeater.count > 0

            Repeater {
                id: repeater
                model: root.controller && !root.controller.privateMode
                       ? root.controller.topSites(6) : []

                delegate: GlassButton {
                    required property var modelData
                    text: modelData.host
                    flat: false
                    onClicked: root.navigationRequested(modelData.url)
                }
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Spacing.large
            visible: root.privacy && root.privacy.totalBlocked > 0

            Repeater {
                model: [
                    { value: root.privacy ? root.privacy.trackersBlocked : 0,
                      label: qsTr("trackers") },
                    { value: root.privacy ? root.privacy.adsBlocked : 0, label: qsTr("ads") },
                    { value: root.privacy ? root.privacy.cookiesBlocked : 0,
                      label: qsTr("cookies") }
                ]

                delegate: Column {
                    required property var modelData
                    spacing: 0

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.value.toLocaleString(Qt.locale(), 'f', 0)
                        color: Colors.text
                        font.family: Typography.family
                        font.pixelSize: Typography.title
                        font.weight: Font.Light
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.label
                        color: Colors.textFaint
                        font.family: Typography.family
                        font.pixelSize: Typography.caption
                        font.letterSpacing: Typography.wideLetterSpacing
                    }
                }
            }
        }
    }

    function focusSearch() {
        search.focusInput();
    }
}
