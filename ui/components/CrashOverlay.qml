// Shown when a tab's render process dies (PLAN.md §30).
//
// Nothing is uploaded. The technical details are shown to the user first, and
// exporting them writes the same text they just read - no URLs, no page
// content, no identifiers.

import QtQuick
import QtQuick.Dialogs
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string report: ""
    property bool detailsVisible: false

    signal reloadRequested()

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
            horizontalAlignment: Text.AlignHCenter
            text: "◍"
            font.pixelSize: 40
            color: Colors.textFaint
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("This page stopped responding")
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.title
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("The process that renders this tab exited. Nothing has been reported to "
                       + "anyone - this browser has no crash uploader.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Spacing.small

            GlassButton {
                text: qsTr("Reload")
                accented: true
                onClicked: root.reloadRequested()
            }

            GlassButton {
                text: root.detailsVisible ? qsTr("Hide technical details")
                                          : qsTr("View technical details")
                onClicked: root.detailsVisible = !root.detailsVisible
            }

            GlassButton {
                text: qsTr("Export report")
                visible: root.detailsVisible
                onClicked: exportDialog.open()
            }
        }

        GlassSurface {
            width: parent.width
            height: reportText.implicitHeight + Spacing.large * 2
            visible: root.detailsVisible
            cornerRadius: Radius.large

            Text {
                id: reportText

                anchors.fill: parent
                anchors.margins: Spacing.large
                text: root.report
                color: Colors.textSubtle
                font.family: Typography.monoFamily
                font.pixelSize: Typography.caption
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                textFormat: Text.PlainText
            }
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: root.detailsVisible
            text: qsTr("An exported report describes this machine's software versions. It "
                       + "contains no addresses and no page content, but read it before you "
                       + "send it to anyone.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }

    FileDialog {
        id: exportDialog

        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("Text files (*.txt)")]
        onAccepted: App.exportReport(selectedFile, root.report)
    }
}
