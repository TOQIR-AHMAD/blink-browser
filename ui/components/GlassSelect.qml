// A dropdown for the settings pages: search provider, theme, cookie policy.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

ComboBox {
    id: control

    implicitWidth: 190
    implicitHeight: 34
    font.family: Typography.family
    font.pixelSize: Typography.body

    background: GlassSurface {
        cornerRadius: Radius.medium
        interactive: true
        hovered: control.hovered
        down: control.pressed
        borderColor: control.activeFocus ? Colors.focusRing : Colors.glassBorder
    }

    contentItem: Text {
        leftPadding: Spacing.medium
        rightPadding: control.indicator.width + Spacing.small
        text: control.displayText
        color: Colors.text
        font: control.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: control.width - width - Spacing.medium
        y: control.topPadding + (control.availableHeight - height) / 2
        text: "⌄"
        color: Colors.textSubtle
        font.pixelSize: Typography.body
    }

    delegate: ItemDelegate {
        id: option

        required property int index
        required property var modelData

        width: control.width
        height: 32
        highlighted: control.highlightedIndex === option.index

        background: Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            radius: Radius.small
            color: option.highlighted ? Colors.hover : "transparent"
        }

        contentItem: Text {
            leftPadding: Spacing.small
            text: control.textRole !== "" && option.modelData[control.textRole] !== undefined
                  ? option.modelData[control.textRole] : option.modelData
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.body
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    popup: Popup {
        y: control.height + Spacing.tiny
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + Spacing.small * 2, 280)
        padding: Spacing.small

        background: GlassSurface {
            cornerRadius: Radius.large
            elevated: true
            fillColor: Colors.glassFillStrong
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds
        }
    }
}
