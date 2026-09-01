// A titled panel: the building block of the settings pages and the privacy
// dashboard. Anything written inside a GlassCard is laid out in a column below
// the heading, including rows produced by a Repeater.
//
// The card declares its own visual tree through `children:` rather than as
// plain child objects. That is deliberate: the default property below routes
// everything a caller writes into the content column, and without the explicit
// assignment the card's own background and layout would be routed there too.
// An earlier version used Control with an inline contentItem, and the caller's
// rows never reached the column at all - they piled up at the top-left corner
// of the card. tests/ui/tst_card.qml is the regression test.

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    property string title: ""
    property string subtitle: ""
    property int padding: Spacing.xlarge
    property int spacingBetweenRows: Spacing.medium

    default property alias cardContent: contentColumn.data

    // What the content column ended up containing, for tests.
    readonly property Item contentItem: contentColumn

    implicitWidth: 520
    implicitHeight: layout.implicitHeight + root.padding * 2
    height: implicitHeight

    children: [
        GlassSurface {
            anchors.fill: parent
            cornerRadius: Radius.xlarge
            elevated: true
        },

        Column {
            id: layout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: root.padding
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

            Column {
                id: contentColumn

                width: parent.width
                spacing: root.spacingBetweenRows
            }
        }
    ]
}
