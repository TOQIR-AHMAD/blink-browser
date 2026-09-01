// Type scale.
//
// Sizes are multiplied by Theme.textScale so the OS-level and in-app text-size
// preferences actually do something (PLAN.md §37).

pragma Singleton

import QtQuick

QtObject {
    id: typography

    property real scale: 1.0

    // The platform UI font, with a readable fallback chain rather than a
    // single hard-coded family.
    readonly property string family: Qt.platform.os === "windows"
        ? "Segoe UI Variable Text, Segoe UI, Inter, sans-serif"
        : (Qt.platform.os === "osx" ? "SF Pro Text, Helvetica Neue, sans-serif"
                                    : "Inter, Noto Sans, DejaVu Sans, sans-serif")
    readonly property string monoFamily: Qt.platform.os === "windows"
        ? "Cascadia Mono, Consolas, monospace"
        : "SF Mono, JetBrains Mono, DejaVu Sans Mono, monospace"

    readonly property int display: Math.round(34 * scale)
    readonly property int title: Math.round(22 * scale)
    readonly property int heading: Math.round(17 * scale)
    readonly property int body: Math.round(14 * scale)
    readonly property int label: Math.round(13 * scale)
    readonly property int caption: Math.round(11 * scale)

    readonly property real tightLetterSpacing: -0.3
    readonly property real wideLetterSpacing: 0.6
}
