// A divider in a GlassMenu: a hairline, inset from the edges, in the same
// border colour as every other seam in the interface.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

MenuSeparator {
    id: control

    padding: Spacing.tiny
    topPadding: Spacing.small
    bottomPadding: Spacing.small

    contentItem: Rectangle {
        implicitWidth: 200
        implicitHeight: 1
        color: Colors.glassBorder
    }
}
