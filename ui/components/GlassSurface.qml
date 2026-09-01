// The base of the design system: a translucent, softly lit panel.
//
// Every floating piece of chrome is one of these. It gives you the background,
// the thin border, the highlight along the top edge, the soft shadow and the
// hover/pressed states from PLAN.md §6; put whatever you like inside it.
//
// A note on the blur: a true backdrop blur has to sample what is behind the
// surface, which only works when "behind" is a single item the surface covers
// entirely. Full-window overlays (GlassDialog, GlassMenu) do exactly that and
// blur properly. The chrome surfaces instead sit on the window's own gradient
// and use translucency plus the highlight to read as glass - which costs
// nothing per frame and cannot go wrong on a machine without a GPU.

import QtQuick
import QtQuick.Effects
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    // Visual
    property int cornerRadius: Radius.large
    property color fillColor: Colors.glassFill
    property color borderColor: Colors.glassBorder
    property int borderWidth: 1
    property bool elevated: false
    property bool showHighlight: true
    property real fillOpacity: 1.0

    // Interaction: set by the control that wraps this surface.
    property bool interactive: false
    property bool hovered: false
    property bool down: false
    property bool selected: false

    // No default-property alias on purpose: children declared inside a
    // GlassSurface are ordinary children of this Item, stacked above the glass
    // body, and can anchor to it directly.

    readonly property color effectiveFill: {
        if (root.selected)
            return Colors.glassFillStrong;
        if (root.interactive && root.down)
            return Qt.tint(root.fillColor, Colors.pressed);
        if (root.interactive && root.hovered)
            return Qt.tint(root.fillColor, Colors.hover);
        return root.fillColor;
    }

    implicitWidth: 120
    implicitHeight: 40

    Rectangle {
        id: body

        anchors.fill: parent
        radius: root.cornerRadius
        color: root.effectiveFill
        opacity: root.fillOpacity
        border.width: root.borderWidth
        border.color: root.selected ? Colors.glassBorderStrong : root.borderColor
        antialiasing: true
        visible: !shadow.visible

        Behavior on color {
            ColorAnimation {
                duration: Theme.instant
                easing.type: Theme.easing
            }
        }

        // The light along the top edge is what makes a flat translucent
        // rectangle read as a pane of glass.
        Rectangle {
            visible: root.showHighlight
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: root.cornerRadius * 0.5
            height: 1
            radius: 1
            color: Colors.glassHighlight
            opacity: Colors.dark ? 0.5 : 0.9
        }
    }

    MultiEffect {
        id: shadow

        anchors.fill: body
        source: body
        visible: root.elevated && Theme.glassEnabled
        shadowEnabled: true
        shadowColor: Colors.shadow
        shadowBlur: Shadows.raisedBlur
        shadowVerticalOffset: Shadows.raisedOffset * 0.5
        shadowOpacity: 1.0
        autoPaddingEnabled: true
    }
}
