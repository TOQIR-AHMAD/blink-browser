// Corner radii. Rounded, but not so rounded that a control loses its shape.

pragma Singleton

import QtQuick

QtObject {
    readonly property int small: 6
    readonly property int medium: 10
    readonly property int large: 14
    readonly property int xlarge: 20
    readonly property int pill: 999
}
