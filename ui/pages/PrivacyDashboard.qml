// The privacy dashboard (PLAN.md §33).
//
// Every number here is a counter held in memory for this session. There is no
// per-site history behind them, they are never written to disk, and they are
// never sent anywhere - which is the point the page makes at the bottom.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Flickable {
    id: root

    required property var privacy
    required property var permissions

    contentWidth: width
    contentHeight: layout.implicitHeight + Spacing.page * 2
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: ScrollBar {}

    Column {
        id: layout

        x: Spacing.page
        y: Spacing.page
        width: Math.min(root.width - Spacing.page * 2, 760)
        spacing: Spacing.xlarge

        Column {
            width: parent.width
            spacing: Spacing.tiny

            Text {
                text: qsTr("Privacy protection")
                color: Colors.text
                font.family: Typography.family
                font.pixelSize: Typography.display
                font.weight: Font.Light
                font.letterSpacing: Typography.tightLetterSpacing
            }

            Text {
                text: qsTr("This session only. Nothing below leaves this device.")
                color: Colors.textSubtle
                font.family: Typography.family
                font.pixelSize: Typography.body
            }
        }

        Flow {
            width: parent.width
            spacing: Spacing.medium

            StatTile {
                value: root.privacy.trackersBlocked
                label: qsTr("TRACKERS BLOCKED")
                accentColor: Colors.secure
            }

            StatTile {
                value: root.privacy.adsBlocked
                label: qsTr("ADS BLOCKED")
                accentColor: Colors.accent
            }

            StatTile {
                value: root.privacy.cookiesBlocked
                label: qsTr("COOKIES BLOCKED")
                accentColor: Colors.privateAccent
            }

            StatTile {
                value: root.privacy.thirdPartyBlocked
                label: qsTr("OTHER REQUESTS BLOCKED")
                accentColor: Colors.insecure
            }

            StatTile {
                value: root.privacy.httpsUpgrades
                label: qsTr("UPGRADED TO HTTPS")
                accentColor: Colors.secure
            }

            StatTile {
                value: root.permissions.grantedCount
                label: qsTr("PERMISSIONS GRANTED")
                accentColor: Colors.text
            }
        }

        GlassCard {
            width: parent.width
            title: qsTr("What this browser keeps")
            subtitle: qsTr("Everything below is what actually happens, not what is promised.")

            Repeater {
                model: root.privacy.storageSummary()

                delegate: Item {
                    required property var modelData

                    width: parent.width
                    height: 44

                    Column {
                        anchors.left: parent.left
                        anchors.right: lifetime.left
                        anchors.rightMargin: Spacing.medium
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            width: parent.width
                            text: modelData.name
                            color: Colors.text
                            font.family: Typography.family
                            font.pixelSize: Typography.body
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: modelData.location
                            color: Colors.textFaint
                            font.family: Typography.family
                            font.pixelSize: Typography.caption
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        id: lifetime
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.lifetime
                        color: Colors.textSubtle
                        font.family: Typography.family
                        font.pixelSize: Typography.label
                    }
                }
            }
        }

        GlassCard {
            width: parent.width
            title: qsTr("Permissions granted this session")
            subtitle: root.permissions.grantedCount + root.permissions.deniedCount > 0
                      ? qsTr("Forgotten when the browser closes.")
                      : qsTr("Nothing has been asked for yet.")

            Repeater {
                model: root.permissions.grants()

                delegate: Row {
                    required property var modelData
                    width: parent.width
                    spacing: Spacing.small

                    Text {
                        width: parent.width * 0.5
                        text: modelData.origin
                        color: Colors.text
                        font.family: Typography.family
                        font.pixelSize: Typography.label
                        elide: Text.ElideMiddle
                    }

                    Text {
                        text: modelData.featureName
                        color: Colors.textSubtle
                        font.family: Typography.family
                        font.pixelSize: Typography.label
                    }

                    Text {
                        text: modelData.allowed ? qsTr("Allowed") : qsTr("Blocked")
                        color: modelData.allowed ? Colors.secure : Colors.danger
                        font.family: Typography.family
                        font.pixelSize: Typography.label
                        font.weight: Font.DemiBold
                    }
                }
            }

            GlassButton {
                text: qsTr("Forget all permission answers")
                visible: root.permissions.grantedCount + root.permissions.deniedCount > 0
                onClicked: root.permissions.clear()
            }
        }

        Row {
            spacing: Spacing.small

            GlassButton {
                text: qsTr("Clear browsing data now")
                onClicked: root.privacy.clearBrowsingDataNow()
            }

            GlassButton {
                text: qsTr("Reset these counters")
                flat: true
                onClicked: root.privacy.resetStatistics()
            }
        }

        Text {
            width: parent.width
            text: qsTr("These counters live in memory. They are not written to disk, not "
                       + "attached to any identifier, and not sent to the developers - there "
                       + "is no server to send them to.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }
}
