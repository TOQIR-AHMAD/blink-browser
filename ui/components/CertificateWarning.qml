// Certificate interstitial.
//
// Defaults to going back. Proceeding is possible only when Chromium says the
// error is overridable, and the wording does not soften what proceeding means
// (PLAN.md §24).

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string host: ""
    property string description: ""
    property bool overridable: false

    signal goBackRequested()
    signal proceedRequested()

    Rectangle {
        anchors.fill: parent
        color: Colors.contentBackground
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Spacing.huge * 2, 560)
        spacing: Spacing.large

        Text {
            width: parent.width
            text: "⛔"
            font.pixelSize: 40
            color: Colors.danger
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            text: qsTr("This connection is not private")
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.title
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: qsTr("The certificate offered by %1 could not be verified, so there is no way "
                       + "to tell whether you are talking to that site or to somebody in "
                       + "between.").arg(root.host)
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            visible: root.description !== ""
            text: root.description
            color: Colors.textFaint
            font.family: Typography.monoFamily
            font.pixelSize: Typography.caption
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Spacing.small

            GlassButton {
                text: qsTr("Go back")
                accented: true
                onClicked: root.goBackRequested()
            }

            GlassButton {
                text: qsTr("Continue anyway")
                danger: true
                visible: root.overridable
                onClicked: root.proceedRequested()
            }
        }

        Text {
            width: parent.width
            visible: !root.overridable
            text: qsTr("This error cannot be overridden.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
