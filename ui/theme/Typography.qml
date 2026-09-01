// Type scale.
//
// Ubuntu and Ubuntu Mono, which ship with the application under the Ubuntu
// Font Licence (ui/assets/fonts/LICENCE-Ubuntu-font.txt) so the interface looks
// the same on a machine that has never seen them. Fonts.qml loads the files;
// if loading fails for any reason the names below fall back to the platform UI
// font rather than to something unreadable.
//
// Sizes are multiplied by Theme.textScale so the OS-level and in-app text-size
// preferences actually do something (PLAN.md §37).

pragma Singleton

import QtQuick

QtObject {
    id: typography

    property real scale: 1.0

    readonly property string family: "Ubuntu, Ubuntu Sans, Segoe UI, Cantarell, sans-serif"
    readonly property string monoFamily: "Ubuntu Mono, Ubuntu Sans Mono, Cascadia Mono, "
                                         + "Consolas, monospace"

    readonly property int display: Math.round(34 * scale)
    readonly property int title: Math.round(22 * scale)
    readonly property int heading: Math.round(17 * scale)
    readonly property int body: Math.round(14 * scale)
    readonly property int label: Math.round(13 * scale)
    readonly property int caption: Math.round(11 * scale)

    // Ubuntu's letterforms are already fairly tight; the display sizes get a
    // little negative tracking, nothing else does.
    readonly property real tightLetterSpacing: -0.2
    readonly property real wideLetterSpacing: 0.5
}
