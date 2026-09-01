// A labelled row in the settings pages: a description on the left, a control
// on the right. Used for the choices that are not simple switches.

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string label: ""
    property string description: ""
    property Item control: null

    implicitHeight: Math.max(textColumn.implicitHeight, controlHolder.implicitHeight)
    implicitWidth: 400

    onControlChanged: {
        if (control)
            control.parent = controlHolder;
    }

    Column {
        id: textColumn

        anchors.left: parent.left
        anchors.right: controlHolder.left
        anchors.rightMargin: Spacing.large
        anchors.verticalCenter: parent.verticalCenter
        spacing: Spacing.hair

        Text {
            width: parent.width
            text: root.label
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            text: root.description
            visible: text !== ""
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }

    Item {
        id: controlHolder

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: root.control ? root.control.implicitWidth : 0
        implicitHeight: root.control ? root.control.implicitHeight : 0
        width: implicitWidth
        height: implicitHeight
    }
}
