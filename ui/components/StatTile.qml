// One big number on the privacy dashboard.

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property int value: 0
    property string label: ""
    property color accentColor: Colors.accent

    implicitWidth: 150
    implicitHeight: 120

    GlassSurface {
        anchors.fill: parent
        cornerRadius: Radius.xlarge
        fillColor: Colors.glassFill
    }

    Column {
        anchors.centerIn: parent
        spacing: Spacing.tiny
        width: parent.width - Spacing.medium * 2

        Text {
            width: parent.width
            text: root.value.toLocaleString(Qt.locale())
            color: root.accentColor
            font.family: Typography.family
            font.pixelSize: Typography.display
            font.weight: Font.Light
            font.letterSpacing: Typography.tightLetterSpacing
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            text: root.label
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.caption
            font.letterSpacing: Typography.wideLetterSpacing
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
