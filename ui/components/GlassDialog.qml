// A modal panel over a blurred copy of the window.
//
// This is the one place a real backdrop blur is practical: the overlay covers
// the whole window, so the blurred copy has an obvious source rectangle and
// stays in step without any coordinate mapping.

import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import PrivacyBrowser.Ui.Theme

Popup {
    id: dialog

    // The item to blur behind the dialog: the window's content, not including
    // this overlay, or the effect would feed on itself.
    property Item backdropSource: null
    property string title: ""
    property string message: ""

    anchors.centerIn: Overlay.overlay
    modal: true
    dim: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: Spacing.xlarge
    implicitWidth: 460

    background: GlassSurface {
        cornerRadius: Radius.xlarge
        elevated: true
        fillColor: Colors.glassFillStrong
        borderColor: Colors.glassBorderStrong
    }

    Overlay.modal: Item {
        ShaderEffectSource {
            id: backdropTexture
            anchors.fill: parent
            sourceItem: dialog.backdropSource
            live: true
            visible: false
            hideSource: false
        }

        MultiEffect {
            anchors.fill: parent
            source: backdropTexture
            visible: Theme.glassEnabled && dialog.backdropSource !== null
            blurEnabled: true
            blur: 1.0
            blurMax: Theme.blurMax
            blurMultiplier: 0.6
        }

        Rectangle {
            anchors.fill: parent
            color: Colors.dark ? Qt.rgba(0, 0, 0, 0.42) : Qt.rgba(0.05, 0.06, 0.12, 0.22)
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: Theme.quick
        }
        NumberAnimation {
            property: "scale"
            from: 0.94
            to: 1.0
            duration: Theme.gentle
            easing.type: Theme.easing
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            to: 0.0
            duration: Theme.instant
        }
    }
}
