// Settings > About.

import QtQuick
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Column {
    id: root

    required property var app

    spacing: Spacing.large

    GlassCard {
        width: parent.width

        Row {
            spacing: Spacing.large

            Image {
                source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/icon.png"
                width: 64
                height: 64
                fillMode: Image.PreserveAspectFit
                sourceSize.width: 128
                sourceSize.height: 128
            }

            Column {
                spacing: Spacing.tiny

                Text {
                    text: qsTr("Privacy Browser")
                    color: Colors.text
                    font.family: Typography.family
                    font.pixelSize: Typography.title
                    font.weight: Font.DemiBold
                }

                Text {
                    text: qsTr("Version %1").arg(root.app.version)
                    color: Colors.textSubtle
                    font.family: Typography.monoFamily
                    font.pixelSize: Typography.label
                }

                Text {
                    text: qsTr("Chromium %1 · Qt %2").arg(root.app.chromiumVersion)
                          .arg(root.app.qtVersion)
                    color: Colors.textFaint
                    font.family: Typography.monoFamily
                    font.pixelSize: Typography.caption
                }
            }
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("What this browser is")

        Text {
            width: parent.width
            text: qsTr("Chromium renders the web, Qt draws the interface, and everything "
                       + "between them is written to keep browsing on this machine. There is "
                       + "no account, no sync, no telemetry and no server belonging to this "
                       + "project - the browser has nowhere to send anything even if it "
                       + "wanted to.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("Updates")

        Text {
            width: parent.width
            text: qsTr("There is no automatic update check, so the browser never contacts "
                       + "anyone to ask whether it is current. Updating means downloading a "
                       + "new build yourself.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("Built on")

        Text {
            width: parent.width
            text: qsTr("Chromium and Qt WebEngine, Qt 6, and the two filter lists written for "
                       + "this project. Their licences ship with the application.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }
    }
}
