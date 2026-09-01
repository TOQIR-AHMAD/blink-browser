// The downloads panel.
//
// The list is this session's only. Closing the browser forgets that a download
// ever happened; the files themselves stay where they were saved, because they
// are the user's (PLAN.md §28).

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

GlassCard {
    id: root

    required property var downloads // pb::downloads::DownloadManager

    signal saveRequested(var item)

    title: qsTr("Downloads")
    subtitle: qsTr("This list is forgotten when the browser closes. The files are not.")

    Text {
        width: parent.width
        visible: root.downloads.count === 0
        text: qsTr("Nothing downloaded yet.")
        color: Colors.textFaint
        font.family: Typography.family
        font.pixelSize: Typography.body
    }

    ListView {
        width: parent.width
        height: Math.min(contentHeight, 320)
        visible: root.downloads.count > 0
        clip: true
        spacing: Spacing.small
        model: root.downloads

        delegate: Item {
            id: row

            required property var download

            width: ListView.view.width
            height: 56

            Column {
                anchors.left: parent.left
                anchors.right: actions.left
                anchors.rightMargin: Spacing.small
                anchors.verticalCenter: parent.verticalCenter
                spacing: Spacing.hair

                Text {
                    width: parent.width
                    text: row.download.fileName
                    color: Colors.text
                    font.family: Typography.family
                    font.pixelSize: Typography.body
                    elide: Text.ElideMiddle
                }

                Text {
                    width: parent.width
                    color: Colors.textSubtle
                    font.family: Typography.family
                    font.pixelSize: Typography.caption
                    elide: Text.ElideRight
                    text: {
                        switch (row.download.state) {
                        case 0:
                            return row.download.totalBytes > 0
                                ? qsTr("%1% of %2").arg(Math.round(row.download.progress * 100))
                                                   .arg(root.formatBytes(row.download.totalBytes))
                                : qsTr("Downloading…");
                        case 1:
                            return qsTr("Finished - choose where to keep it");
                        case 2:
                            return row.download.savePath;
                        case 3:
                            return qsTr("Cancelled");
                        default:
                            return qsTr("Failed");
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 3
                    radius: 1.5
                    color: Colors.glassFill
                    visible: row.download.state === 0

                    Rectangle {
                        width: parent.width * row.download.progress
                        height: parent.height
                        radius: parent.radius
                        color: Colors.accent
                    }
                }
            }

            Row {
                id: actions

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: Spacing.tiny

                GlassButton {
                    width: 30
                    height: 30
                    flat: true
                    glyph: "💾"
                    visible: row.download.state === 1
                    tooltip: qsTr("Save as…")
                    onClicked: root.saveRequested(row.download)
                }

                GlassButton {
                    width: 30
                    height: 30
                    flat: true
                    glyph: "📁"
                    visible: row.download.state === 2
                    tooltip: qsTr("Show in folder")
                    onClicked: root.downloads.openContainingFolder(row.download)
                }

                GlassButton {
                    width: 30
                    height: 30
                    flat: true
                    glyph: "✕"
                    tooltip: row.download.state === 0 ? qsTr("Cancel") : qsTr("Remove from list")
                    onClicked: root.downloads.discard(row.download)
                }
            }
        }
    }

    Row {
        width: parent.width
        layoutDirection: Qt.RightToLeft
        visible: root.downloads.count > 0

        GlassButton {
            text: qsTr("Clear finished")
            flat: true
            onClicked: root.downloads.clearFinished()
        }
    }

    function formatBytes(bytes) {
        if (bytes < 1024)
            return qsTr("%1 B").arg(bytes);
        if (bytes < 1024 * 1024)
            return qsTr("%1 kB").arg((bytes / 1024).toFixed(1));
        if (bytes < 1024 * 1024 * 1024)
            return qsTr("%1 MB").arg((bytes / (1024 * 1024)).toFixed(1));
        return qsTr("%1 GB").arg((bytes / (1024 * 1024 * 1024)).toFixed(2));
    }
}
