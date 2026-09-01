// Address-bar completions.
//
// Every row comes from pb::browser::SessionHistory - pages visited in this
// session, in this process, in a non-private window. Nothing is requested from
// the network to build this list (PLAN.md §10).

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    required property var controller
    property int maximumRows: 6
    property int selectedIndex: -1
    readonly property int count: list.count

    signal activated(url target)

    implicitHeight: Math.min(count, maximumRows) * 40 + Spacing.small * 2
    height: implicitHeight
    visible: count > 0
    z: 100

    function refresh(query) {
        if (!root.controller || query === "") {
            list.model = [];
            root.selectedIndex = -1;
            return;
        }
        list.model = root.controller.completions(query, root.maximumRows);
        root.selectedIndex = -1;
    }

    function close() {
        list.model = [];
        root.selectedIndex = -1;
    }

    function moveSelection(delta) {
        if (count === 0)
            return;
        root.selectedIndex = Math.max(-1, Math.min(count - 1, root.selectedIndex + delta));
    }

    function selectedUrl() {
        if (root.selectedIndex < 0 || root.selectedIndex >= count)
            return "";
        return list.model[root.selectedIndex].url;
    }

    GlassSurface {
        anchors.fill: parent
        cornerRadius: Radius.large
        elevated: true
        fillColor: Colors.glassFillStrong
    }

    ListView {
        id: list

        anchors.fill: parent
        anchors.margins: Spacing.small
        clip: true
        interactive: count > root.maximumRows
        model: []

        delegate: AbstractButton {
            id: row

            required property int index
            required property var modelData

            width: ListView.view.width
            height: 40
            hoverEnabled: true

            onClicked: root.activated(row.modelData.url)

            background: Rectangle {
                radius: Radius.medium
                color: row.hovered || root.selectedIndex === row.index
                       ? Colors.hover : "transparent"
            }

            contentItem: Row {
                spacing: Spacing.small
                leftPadding: Spacing.small

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "🕘"
                    font.pixelSize: Typography.label
                    color: Colors.textFaint
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: row.width - 60

                    Text {
                        width: parent.width
                        text: row.modelData.title !== "" ? row.modelData.title
                                                         : row.modelData.host
                        color: Colors.text
                        font.family: Typography.family
                        font.pixelSize: Typography.label
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: row.modelData.url
                        color: Colors.textFaint
                        font.family: Typography.family
                        font.pixelSize: Typography.caption
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
