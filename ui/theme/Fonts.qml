// Loads the bundled Ubuntu font faces.
//
// The files live in ui/assets/fonts and are compiled into the binary, so the
// interface uses Ubuntu on a machine that does not have it installed. They are
// redistributed under the Ubuntu Font Licence, whose text ships beside them.

pragma Singleton

import QtQuick

QtObject {
    readonly property FontLoader regular: FontLoader {
        source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/fonts/Ubuntu-R.ttf"
    }
    readonly property FontLoader light: FontLoader {
        source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/fonts/Ubuntu-L.ttf"
    }
    readonly property FontLoader medium: FontLoader {
        source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/fonts/Ubuntu-M.ttf"
    }
    readonly property FontLoader bold: FontLoader {
        source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/fonts/Ubuntu-B.ttf"
    }
    readonly property FontLoader mono: FontLoader {
        source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/fonts/UbuntuMono-R.ttf"
    }
    readonly property FontLoader monoBold: FontLoader {
        source: "qrc:/qt/qml/PrivacyBrowser/Ui/assets/fonts/UbuntuMono-B.ttf"
    }

    // True once the family is available under its own name.
    readonly property bool ready: regular.status === FontLoader.Ready
}
