// The spacing scale. Every margin, padding and gap in the UI is one of these,
// which is what keeps the layout feeling deliberate rather than assembled.

pragma Singleton

import QtQuick

QtObject {
    readonly property int hair: 2
    readonly property int tiny: 4
    readonly property int small: 8
    readonly property int medium: 12
    readonly property int large: 16
    readonly property int xlarge: 24
    readonly property int xxlarge: 32
    readonly property int huge: 48
    readonly property int page: 40
}
