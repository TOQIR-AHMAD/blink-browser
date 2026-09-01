// A button on a glass surface.
//
// Built on AbstractButton so keyboard activation, focus handling and the
// accessibility role come from Qt rather than being reimplemented badly
// (PLAN.md §37).

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

AbstractButton {
    id: control

    // "text" is the label. Set "glyph" instead for an icon-style button.
    property string glyph: ""
    property bool flat: false
    property bool accented: false
    property bool danger: false
    property int cornerRadius: Radius.medium
    property string tooltip: ""

    implicitWidth: Math.max(glyph !== "" ? 34 : 0,
                            label.implicitWidth + Spacing.large * 2)
    implicitHeight: 34

    padding: Spacing.small
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    opacity: enabled ? 1.0 : 0.45

    Accessible.role: Accessible.Button
    Accessible.name: text !== "" ? text : tooltip
    Accessible.onPressAction: control.clicked()

    background: GlassSurface {
        cornerRadius: control.cornerRadius
        interactive: true
        hovered: control.hovered
        down: control.down
        borderWidth: control.flat ? 0 : 1
        fillColor: {
            if (control.accented)
                return Colors.accentMuted;
            if (control.flat)
                return "transparent";
            return Colors.glassFill;
        }
        borderColor: control.accented ? Colors.accent : Colors.glassBorder
        showHighlight: !control.flat

        // Focus ring, drawn only for keyboard focus.
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: control.cornerRadius + 2
            color: "transparent"
            border.width: 2
            border.color: Colors.focusRing
            visible: control.visualFocus
        }
    }

    contentItem: Text {
        id: label

        text: control.glyph !== "" ? control.glyph : control.text
        font.family: Typography.family
        font.pixelSize: control.glyph !== "" ? Typography.heading : Typography.body
        font.weight: control.accented ? Font.DemiBold : Font.Normal
        color: {
            if (control.danger)
                return Colors.danger;
            if (control.accented)
                return Colors.accent;
            return Colors.text;
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color {
            ColorAnimation {
                duration: Theme.instant
            }
        }
    }

    ToolTip.visible: control.tooltip !== "" && control.hovered
    ToolTip.delay: 600
    ToolTip.text: control.tooltip

    scale: control.down && !Theme.reducedMotion ? 0.96 : 1.0
    Behavior on scale {
        NumberAnimation {
            duration: Theme.instant
            easing.type: Theme.easing
        }
    }
}
