// The palette, in one place.
//
// Two complete sets - light and dark - selected by Theme.dark. Every colour
// used anywhere in the UI comes from here; nothing hard-codes a hex value, so
// a change to the identity is a change to this file.

pragma Singleton

import QtQuick

QtObject {
    id: colors

    // Set by Theme, which is what the rest of the UI talks to.
    property bool dark: true

    // Window and page surfaces -------------------------------------------
    readonly property color background: dark ? "#0A0A0F" : "#EEF0F6"
    readonly property color backgroundElevated: dark ? "#12121A" : "#F7F8FC"
    readonly property color contentBackground: dark ? "#0E0E14" : "#FFFFFF"

    // A gradient behind the window chrome. Subtle: it should read as depth,
    // not as decoration.
    readonly property color auroraOne: dark ? "#1B2559" : "#DCE3FF"
    readonly property color auroraTwo: dark ? "#2A1B4A" : "#EADDFF"
    readonly property color auroraThree: dark ? "#0A2E3D" : "#DDF2FF"

    // Glass ---------------------------------------------------------------
    readonly property color glassFill: dark ? Qt.rgba(1, 1, 1, 0.07)
                                            : Qt.rgba(1, 1, 1, 0.66)
    readonly property color glassFillStrong: dark ? Qt.rgba(1, 1, 1, 0.12)
                                                  : Qt.rgba(1, 1, 1, 0.82)
    readonly property color glassHighlight: dark ? Qt.rgba(1, 1, 1, 0.16)
                                                 : Qt.rgba(1, 1, 1, 0.95)
    readonly property color glassBorder: dark ? Qt.rgba(1, 1, 1, 0.14)
                                              : Qt.rgba(0, 0, 0, 0.08)
    readonly property color glassBorderStrong: dark ? Qt.rgba(1, 1, 1, 0.24)
                                                    : Qt.rgba(0, 0, 0, 0.14)
    readonly property color shadow: dark ? Qt.rgba(0, 0, 0, 0.55)
                                         : Qt.rgba(0.05, 0.07, 0.15, 0.18)

    // Interaction ---------------------------------------------------------
    readonly property color hover: dark ? Qt.rgba(1, 1, 1, 0.10)
                                        : Qt.rgba(0, 0, 0, 0.05)
    readonly property color pressed: dark ? Qt.rgba(1, 1, 1, 0.16)
                                          : Qt.rgba(0, 0, 0, 0.09)
    readonly property color selection: dark ? Qt.rgba(0.43, 0.55, 1.0, 0.35)
                                            : Qt.rgba(0.30, 0.39, 0.82, 0.28)

    // Text ----------------------------------------------------------------
    readonly property color text: dark ? "#F4F4F8" : "#15161C"
    readonly property color textSubtle: dark ? "#A8A8B6" : "#5C5F6B"
    readonly property color textFaint: dark ? "#70707E" : "#8A8D99"
    readonly property color textOnAccent: "#FFFFFF"

    // Meaning -------------------------------------------------------------
    readonly property color accent: dark ? "#7C93FF" : "#4356C9"
    readonly property color accentMuted: dark ? Qt.rgba(0.49, 0.58, 1.0, 0.18)
                                              : Qt.rgba(0.26, 0.34, 0.79, 0.12)
    readonly property color secure: dark ? "#5BD6A0" : "#0F8A5F"
    readonly property color insecure: dark ? "#F2B45C" : "#A96208"
    readonly property color danger: dark ? "#FF7B7B" : "#C03434"
    readonly property color privateAccent: dark ? "#C79BFF" : "#6D3FBF"

    readonly property color focusRing: dark ? Qt.rgba(0.49, 0.58, 1.0, 0.85)
                                            : Qt.rgba(0.26, 0.34, 0.79, 0.75)
}
