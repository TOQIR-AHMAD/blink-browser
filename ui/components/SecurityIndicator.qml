// The lock (or the warning) at the left of the address bar.
//
// It reports what Chromium told us about the connection and nothing more: a
// browser that shows a reassuring lock it has not verified is worse than one
// that shows nothing (PLAN.md §24).

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    // Matches pb::tabs::Tab::SecurityLevel.
    property int level: 0
    property string issue: ""

    readonly property bool secure: level === 3
    readonly property bool warning: level === 2 || level === 4

    implicitWidth: 20
    implicitHeight: 20

    Text {
        id: glyph

        anchors.centerIn: parent
        font.pixelSize: Typography.body
        text: {
            switch (root.level) {
            case 1:
                return "⚙"; // a page the browser itself provides
            case 2:
                return "⚠"; // plain http
            case 3:
                return "🔒";
            case 4:
                return "⛔"; // certificate problem accepted by the user
            default:
                return "○";
            }
        }
        color: {
            if (root.level === 3)
                return Colors.secure;
            if (root.level === 2)
                return Colors.insecure;
            if (root.level === 4)
                return Colors.danger;
            return Colors.textFaint;
        }
    }

    ToolTip.visible: hover.hovered
    ToolTip.delay: 400
    ToolTip.text: {
        switch (root.level) {
        case 1:
            return qsTr("A page provided by the browser itself");
        case 2:
            return qsTr("Not secure - this page was loaded over plain HTTP, so anyone on the "
                        + "network can read and change it");
        case 3:
            return qsTr("Encrypted connection with a valid certificate. This says nothing about "
                        + "what the site does with what you send it.");
        case 4:
            return root.issue !== ""
                    ? qsTr("Certificate problem: %1").arg(root.issue)
                    : qsTr("This site's certificate could not be verified");
        default:
            return qsTr("Nothing loaded yet");
        }
    }

    HoverHandler {
        id: hover
    }
}
