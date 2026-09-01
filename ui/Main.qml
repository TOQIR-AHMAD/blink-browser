// Phase 0: an empty window that proves the Qt/QML shell starts and shuts down.
// The tab bar, navigation bar and web content are added in Phases 1-3.

import QtQuick

Window {
    id: root

    width: 1200
    height: 800
    minimumWidth: 480
    minimumHeight: 360
    visible: true
    title: qsTr("Privacy Browser")
    color: "#101014"

    Text {
        anchors.centerIn: parent
        text: qsTr("Privacy Browser")
        color: "#f5f5f7"
        font.pixelSize: 20
        font.letterSpacing: 0.5
    }
}
