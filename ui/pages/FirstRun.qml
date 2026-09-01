// The welcome screen (PLAN.md §51).
//
// It states what the browser does by default and then gets out of the way.
// There is nothing to accept, nothing to sign in to, and no option here that
// makes the browser collect more - because there is no such option anywhere.

import QtQuick
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    signal dismissed()

    Rectangle {
        anchors.fill: parent
        color: Colors.background

        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: Colors.auroraTwo }
                GradientStop { position: 0.6; color: Colors.background }
                GradientStop { position: 1.0; color: Colors.auroraOne }
            }
            opacity: 0.7
        }
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Spacing.huge * 2, 560)
        spacing: Spacing.xlarge

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/icon.png"
            width: 72
            height: 72
            sourceSize.width: 144
            sourceSize.height: 144
            fillMode: Image.PreserveAspectFit
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Welcome")
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.display
            font.weight: Font.Light
            font.letterSpacing: Typography.tightLetterSpacing
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("This browser is built to keep your browsing on this machine.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }

        GlassCard {
            width: parent.width
            title: qsTr("By default")

            Repeater {
                model: [
                    qsTr("No browsing history is saved"),
                    qsTr("No cookies survive the session"),
                    qsTr("No telemetry, analytics or crash uploads"),
                    qsTr("No account, and no cloud to sync with"),
                    qsTr("No advertising or installation identifier"),
                    qsTr("Trackers and ads are blocked before the request is made"),
                    qsTr("Camera, microphone and location always ask first")
                ]

                delegate: Row {
                    required property string modelData
                    width: parent.width
                    spacing: Spacing.small

                    Text {
                        text: "✓"
                        color: Colors.secure
                        font.family: Typography.family
                        font.pixelSize: Typography.body
                    }

                    Text {
                        width: parent.width - Spacing.large
                        text: modelData
                        color: Colors.text
                        font.family: Typography.family
                        font.pixelSize: Typography.body
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("What a site, your DNS provider and your network can still see is "
                       + "described in Settings › Privacy. This browser does not claim to make "
                       + "you anonymous.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }

        GlassButton {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Start browsing")
            accented: true
            onClicked: root.dismissed()
        }
    }
}
