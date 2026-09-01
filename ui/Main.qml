// The root object.
//
// It owns nothing visible: it binds the theme to the user's settings and
// creates one BrowserWindow per window the C++ side reports. Closing the last
// window ends the process, which is what runs the session cleanup.

import QtQuick
import PrivacyBrowser.Ui.Theme

QtObject {
    id: root

    readonly property var settings: App.settings

    property Instantiator windows: Instantiator {
        model: App.browser.windows

        delegate: BrowserWindow {
            required property var controller
            windowController: controller
        }
    }

    // Theme follows the settings; the settings' "system" option follows the
    // platform's colour scheme.
    property Binding themeMode: Binding {
        target: Theme
        property: "mode"
        value: root.settings.theme
    }

    property Binding themeMotion: Binding {
        target: Theme
        property: "reducedMotion"
        value: root.settings.reducedMotion
    }

    property Binding themeGlass: Binding {
        target: Theme
        property: "glassEnabled"
        value: root.settings.glassEffects
    }

    property Binding themeScale: Binding {
        target: Theme
        property: "textScale"
        value: root.settings.textScale
    }

    property Binding themeSystemScheme: Binding {
        target: Theme
        property: "systemPrefersDark"
        value: Application.styleHints.colorScheme === Qt.ColorScheme.Dark
    }

    Component.onCompleted: {
        // The first window. Everything after this one is created from the UI.
        if (App.browser.windowCount === 0)
            App.browser.createWindow(false);
    }
}
