// A single-line text field on glass.
//
// Built on TextInput rather than Controls' TextField so the visuals are
// entirely ours and no platform style can reintroduce a native look, or a
// behaviour the browser does not want (there is no undo history to leak, no
// spell-check service to contact).

import QtQuick
import PrivacyBrowser.Ui.Theme

FocusScope {
    id: root

    property alias text: input.text
    property alias placeholderText: placeholder.text
    property alias echoMode: input.echoMode
    property alias readOnly: input.readOnly
    property alias inputItem: input
    property alias horizontalAlignment: input.horizontalAlignment
    property int cornerRadius: Radius.pill
    property int leftPadding: Spacing.medium
    property int rightPadding: Spacing.medium
    property bool selectAllOnFocus: false

    // Optional decorations at either end.
    property alias leading: leadingArea.data
    property alias trailing: trailingArea.data

    signal accepted(string text)
    signal escaped()
    signal editingStarted()

    implicitHeight: 38
    implicitWidth: 240

    function selectAllText() {
        input.selectAll();
    }

    function focusInput() {
        input.forceActiveFocus();
    }

    GlassSurface {
        anchors.fill: parent
        cornerRadius: root.cornerRadius
        fillColor: input.activeFocus ? Colors.glassFillStrong : Colors.glassFill
        borderColor: input.activeFocus ? Colors.focusRing : Colors.glassBorder
        borderWidth: input.activeFocus ? 2 : 1
    }

    Row {
        id: leadingArea
        anchors.left: parent.left
        anchors.leftMargin: root.leftPadding
        anchors.verticalCenter: parent.verticalCenter
        spacing: Spacing.small
    }

    Row {
        id: trailingArea
        anchors.right: parent.right
        anchors.rightMargin: root.rightPadding
        anchors.verticalCenter: parent.verticalCenter
        spacing: Spacing.tiny
    }

    TextInput {
        id: input

        anchors.left: leadingArea.right
        anchors.leftMargin: leadingArea.width > 0 ? Spacing.small : 0
        anchors.right: trailingArea.left
        anchors.rightMargin: trailingArea.width > 0 ? Spacing.small : 0
        anchors.verticalCenter: parent.verticalCenter

        focus: true
        clip: true
        color: Colors.text
        selectionColor: Colors.selection
        selectedTextColor: Colors.text
        font.family: Typography.family
        font.pixelSize: Typography.body
        selectByMouse: true
        activeFocusOnTab: true

        Accessible.role: Accessible.EditableText
        Accessible.name: root.placeholderText

        onActiveFocusChanged: {
            if (activeFocus) {
                if (root.selectAllOnFocus)
                    selectAll();
                root.editingStarted();
            }
        }
        onAccepted: root.accepted(text)

        Keys.onEscapePressed: function (event) {
            root.escaped();
            event.accepted = true;
        }
    }

    Text {
        id: placeholder

        anchors.fill: input
        verticalAlignment: Text.AlignVCenter
        visible: input.text.length === 0 && !input.activeFocus
        color: Colors.textFaint
        font.family: Typography.family
        font.pixelSize: Typography.body
        elide: Text.ElideRight
    }
}
