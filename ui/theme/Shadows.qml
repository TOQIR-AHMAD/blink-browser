// Shadow presets for MultiEffect. Soft and low-contrast: a glass surface
// should look like it is floating a few millimetres above the page, not
// hovering over a spotlight.

pragma Singleton

import QtQuick

QtObject {
    readonly property real ambientBlur: 0.55
    readonly property real ambientOffset: 4
    readonly property real ambientScale: 1.0

    readonly property real raisedBlur: 0.9
    readonly property real raisedOffset: 12
    readonly property real raisedScale: 1.0

    readonly property real overlayBlur: 1.0
    readonly property real overlayOffset: 24
    readonly property real overlayScale: 1.0
}
