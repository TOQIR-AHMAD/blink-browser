// The shield in the toolbar: how much this session has blocked, and a way in
// to the privacy dashboard.
//
// The number is a session total held in memory. There is no per-site
// breakdown because the browser does not keep one (PLAN.md §33).

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

AbstractButton {
    id: control

    property int blockedCount: 0
    property bool privateWindow: false

    implicitWidth: row.implicitWidth + Spacing.medium * 2
    implicitHeight: 30
    hoverEnabled: true

    Accessible.role: Accessible.Button
    Accessible.name: qsTr("Privacy protection: %1 blocked this session").arg(blockedCount)

    background: GlassSurface {
        cornerRadius: Radius.pill
        interactive: true
        hovered: control.hovered
        down: control.down
        fillColor: control.privateWindow ? Qt.rgba(0.78, 0.61, 1.0, 0.16) : Colors.glassFill
        borderColor: control.privateWindow ? Colors.privateAccent : Colors.glassBorder
    }

    contentItem: Row {
        id: row
        spacing: Spacing.tiny

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: control.privateWindow ? "🕶" : "🛡"
            font.pixelSize: Typography.body
            color: control.privateWindow ? Colors.privateAccent : Colors.secure
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: control.blockedCount > 0 ? control.blockedCount.toLocaleString(Qt.locale())
                                           : qsTr("Protected")
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.label
            font.weight: Font.DemiBold
        }
    }

    ToolTip.visible: control.hovered
    ToolTip.delay: 500
    ToolTip.text: control.privateWindow
                  ? qsTr("Private window - nothing from it is recorded, not even in this session")
                  : qsTr("%1 trackers, ads and cookies blocked this session").arg(control.blockedCount)
}
