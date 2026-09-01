// A dropdown menu on glass.
//
// Rows are GlassMenuItem, separators GlassMenuSeparator: Menu's own delegate
// property does not reach MenuItems that are written out by hand.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Menu {
    id: menu

    implicitWidth: 240
    padding: Spacing.small
    margins: Spacing.small

    background: GlassSurface {
        cornerRadius: Radius.large
        elevated: true
        fillColor: Colors.glassFillStrong
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: Theme.instant
        }
        NumberAnimation {
            property: "scale"
            from: 0.96
            to: 1.0
            duration: Theme.quick
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
