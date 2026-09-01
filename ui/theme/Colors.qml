// The palette, in one place.
//
// Ubuntu's Yaru colours, dark and light, selected by Theme.dark. Every colour
// used anywhere in the UI comes from here; nothing hard-codes a hex value, so
// the visual identity is this file.
//
// The reference points are Canonical's own: Ubuntu orange #E95420 as the
// accent, aubergine #772953 for the private-window accent, and Yaru's warm
// neutral greys for the surfaces. Surfaces are flat and separated by hairline
// borders rather than translucency - GNOME/Yaru does not do frosted glass, and
// following that is the point of the theme.

pragma Singleton

import QtQuick

QtObject {
    id: colors

    // Set by Theme, which is what the rest of the UI talks to.
    property bool dark: true

    // Window and page surfaces -------------------------------------------
    readonly property color background: dark ? "#1E1E1E" : "#FAFAFA"
    readonly property color backgroundElevated: dark ? "#303030" : "#F2F2F2"
    readonly property color contentBackground: dark ? "#242424" : "#FFFFFF"

    // Yaru is flat: the three "aurora" stops are the window colour, so the
    // gradient the window paints resolves to a single tone.
    readonly property color auroraOne: background
    readonly property color auroraTwo: dark ? "#2A2226" : "#F7F2F5" // a hint of aubergine
    readonly property color auroraThree: background

    // Surfaces ------------------------------------------------------------
    readonly property color glassFill: dark ? "#303030" : "#FFFFFF"
    readonly property color glassFillStrong: dark ? "#3A3A3A" : "#FFFFFF"
    readonly property color glassHighlight: dark ? Qt.rgba(1, 1, 1, 0.06)
                                                 : Qt.rgba(1, 1, 1, 0.9)
    readonly property color glassBorder: dark ? Qt.rgba(1, 1, 1, 0.12)
                                              : Qt.rgba(0, 0, 0, 0.12)
    readonly property color glassBorderStrong: dark ? Qt.rgba(1, 1, 1, 0.22)
                                                    : Qt.rgba(0, 0, 0, 0.22)
    readonly property color shadow: dark ? Qt.rgba(0, 0, 0, 0.45)
                                         : Qt.rgba(0, 0, 0, 0.14)

    // Interaction ---------------------------------------------------------
    readonly property color hover: dark ? Qt.rgba(1, 1, 1, 0.08)
                                        : Qt.rgba(0, 0, 0, 0.06)
    readonly property color pressed: dark ? Qt.rgba(1, 1, 1, 0.14)
                                          : Qt.rgba(0, 0, 0, 0.11)
    readonly property color selection: dark ? Qt.rgba(0.91, 0.33, 0.13, 0.40)
                                            : Qt.rgba(0.91, 0.33, 0.13, 0.28)

    // Text ----------------------------------------------------------------
    readonly property color text: dark ? "#FFFFFF" : "#111111"
    readonly property color textSubtle: dark ? Qt.rgba(1, 1, 1, 0.70)
                                             : Qt.rgba(0, 0, 0, 0.70)
    readonly property color textFaint: dark ? Qt.rgba(1, 1, 1, 0.45)
                                            : Qt.rgba(0, 0, 0, 0.45)
    readonly property color textOnAccent: "#FFFFFF"

    // Meaning -------------------------------------------------------------
    // Ubuntu orange, and Yaru's semantic colours.
    readonly property color accent: "#E95420"
    readonly property color accentMuted: dark ? Qt.rgba(0.91, 0.33, 0.13, 0.20)
                                              : Qt.rgba(0.91, 0.33, 0.13, 0.14)
    readonly property color secure: dark ? "#3EB34F" : "#0E8420"
    readonly property color insecure: dark ? "#F9BC60" : "#C7620C"
    readonly property color danger: dark ? "#E86581" : "#C7162B"
    // Canonical aubergine, for private windows.
    readonly property color privateAccent: dark ? "#B07CA8" : "#772953"

    readonly property color focusRing: accent
}
