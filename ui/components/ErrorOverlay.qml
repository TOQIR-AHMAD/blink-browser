// The page shown when a load fails.
//
// Deliberately plain about what went wrong, and it never sends the failure
// anywhere: there is no error-reporting endpoint in this browser.

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string host: ""
    property int errorCode: 0
    property string errorText: ""
    property bool httpsFallbackOffered: false

    signal retryRequested()
    signal httpFallbackRequested()

    Rectangle {
        anchors.fill: parent
        color: Colors.contentBackground
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Spacing.huge * 2, 520)
        spacing: Spacing.large

        Text {
            width: parent.width
            text: "⚠"
            font.pixelSize: 40
            color: Colors.insecure
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            text: root.host !== "" ? qsTr("Could not reach %1").arg(root.host)
                                   : qsTr("Could not load this page")
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.title
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: root.errorText !== "" ? root.errorText : qsTr("The connection failed.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            visible: root.errorCode !== 0
            text: qsTr("Error code %1").arg(root.errorCode)
            color: Colors.textFaint
            font.family: Typography.monoFamily
            font.pixelSize: Typography.caption
            horizontalAlignment: Text.AlignHCenter
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Spacing.small

            GlassButton {
                text: qsTr("Try again")
                accented: true
                onClicked: root.retryRequested()
            }

            GlassButton {
                text: qsTr("Continue without HTTPS")
                visible: root.httpsFallbackOffered
                onClicked: root.httpFallbackRequested()
            }
        }

        Text {
            width: parent.width
            visible: root.httpsFallbackOffered
            text: qsTr("This site was tried over HTTPS first. Continuing without it means "
                       + "anyone on the network can read and change this page.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
