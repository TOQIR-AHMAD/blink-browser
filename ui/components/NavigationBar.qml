// Back, forward, address bar, privacy badge, menu.

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    required property var controller
    property var tab: null
    property int blockedCount: 0

    signal privacyRequested()
    signal menuRequested(Item anchorItem)
    signal navigationRequested(url target)

    implicitHeight: 46

    function focusAddress() {
        addressBar.focusAddress();
    }

    Row {
        id: leftControls

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: Spacing.tiny

        GlassButton {
            width: 32
            height: 32
            flat: true
            glyph: "‹"
            tooltip: qsTr("Back (Alt+Left)")
            enabled: root.tab && root.tab.canGoBack
            onClicked: root.tab.goBack()
        }

        GlassButton {
            width: 32
            height: 32
            flat: true
            glyph: "›"
            tooltip: qsTr("Forward (Alt+Right)")
            enabled: root.tab && root.tab.canGoForward
            onClicked: root.tab.goForward()
        }
    }

    AddressBar {
        id: addressBar

        anchors.left: leftControls.right
        anchors.leftMargin: Spacing.medium
        anchors.right: rightControls.left
        anchors.rightMargin: Spacing.medium
        anchors.verticalCenter: parent.verticalCenter
        controller: root.controller
        tab: root.tab

        onNavigationRequested: function (target) {
            root.navigationRequested(target);
        }
    }

    Row {
        id: rightControls

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: Spacing.small

        PrivacyBadge {
            anchors.verticalCenter: parent.verticalCenter
            blockedCount: root.blockedCount
            privateWindow: root.controller && root.controller.privateMode
            onClicked: root.privacyRequested()
        }

        GlassButton {
            id: menuButton
            anchors.verticalCenter: parent.verticalCenter
            width: 32
            height: 32
            flat: true
            glyph: "⋯"
            tooltip: qsTr("Menu")
            onClicked: root.menuRequested(menuButton)
        }
    }
}
