// A titled panel: the building block of the settings pages and the privacy
// dashboard.
//
// Built on Control so that anything written inside a GlassCard lands in the
// content column, below the heading, without the card having to redefine a
// default property.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Control {
    id: root

    property string title: ""
    property string subtitle: ""
    property int spacingBetweenRows: Spacing.medium

    padding: Spacing.xlarge
    implicitWidth: 520

    background: GlassSurface {
        cornerRadius: Radius.xlarge
        elevated: true
    }

    contentItem: Column {
        spacing: root.spacingBetweenRows

        Column {
            width: parent.width
            spacing: Spacing.hair
            visible: root.title !== "" || root.subtitle !== ""

            Text {
                width: parent.width
                text: root.title
                visible: text !== ""
                color: Colors.text
                font.family: Typography.family
                font.pixelSize: Typography.heading
                font.weight: Font.DemiBold
                font.letterSpacing: Typography.tightLetterSpacing
                wrapMode: Text.WordWrap
            }

            Text {
                width: parent.width
                text: root.subtitle
                visible: text !== ""
                color: Colors.textSubtle
                font.family: Typography.family
                font.pixelSize: Typography.label
                wrapMode: Text.WordWrap
            }
        }
    }
}
