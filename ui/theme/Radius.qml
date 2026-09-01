// Corner radii, following Yaru: buttons and rows are gently rounded, windows
// and cards a little more, entries fully rounded.

pragma Singleton

import QtQuick

QtObject {
    readonly property int small: 6
    readonly property int medium: 8
    readonly property int large: 12
    readonly property int xlarge: 16
    readonly property int pill: 999
}
