// The one object the rest of the UI talks to about appearance.
//
// It holds the state (dark or light, motion, text scale, whether glass effects
// are on) and forwards it to the palette and the type scale. Main.qml binds it
// to the user's settings; nothing else writes to it.

pragma Singleton

import QtQuick

QtObject {
    id: theme

    // 0 = follow the system, 1 = light, 2 = dark. Matches
    // SettingsController::ThemeValue.
    property int mode: 0
    property bool systemPrefersDark: true
    property bool reducedMotion: false
    property bool glassEnabled: true
    property real textScale: 1.0

    readonly property bool dark: mode === 2 || (mode === 0 && systemPrefersDark)

    // Animation durations. Every transition in the UI uses one of these, so
    // "reduce motion" is a single switch rather than a hunt through the code
    // (PLAN.md §35, §37).
    readonly property int instant: reducedMotion ? 0 : 90
    readonly property int quick: reducedMotion ? 0 : 160
    readonly property int gentle: reducedMotion ? 0 : 240
    readonly property int slow: reducedMotion ? 0 : 380

    readonly property int easing: Easing.OutCubic
    readonly property int springEasing: reducedMotion ? Easing.Linear : Easing.OutBack

    // Blur strength for the frosted surfaces; 0 turns the effect off entirely.
    readonly property real blurAmount: glassEnabled ? 1.0 : 0.0
    readonly property int blurMax: 48

    onDarkChanged: Colors.dark = theme.dark
    onTextScaleChanged: Typography.scale = theme.textScale
    Component.onCompleted: {
        Colors.dark = theme.dark;
        Typography.scale = theme.textScale;
    }
}
