// The tab strip.
//
// Tabs share the available width, down to a minimum, and reorder live while
// being dragged: the model moves under the pointer instead of the tab being
// dragged as a floating copy, which keeps the strip and the model in step at
// all times.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    required property var tabs // pb::tabs::TabModel
    property int minimumTabWidth: 92
    property int maximumTabWidth: 220

    implicitHeight: 38

    readonly property int tabWidth: {
        if (!tabs || tabs.count === 0)
            return maximumTabWidth;
        const available = width - newTabButton.width - Spacing.small * 2;
        const each = Math.floor(available / tabs.count) - list.spacing;
        return Math.max(minimumTabWidth, Math.min(maximumTabWidth, each));
    }

    ListView {
        id: list

        anchors.left: parent.left
        anchors.right: newTabButton.left
        anchors.rightMargin: Spacing.small
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height
        orientation: ListView.Horizontal
        spacing: Spacing.tiny
        clip: true
        model: root.tabs
        currentIndex: root.tabs ? root.tabs.currentIndex : -1
        boundsBehavior: Flickable.StopAtBounds

        // Wheel over the strip walks the tabs, which is what people expect.
        WheelHandler {
            onWheel: function (event) {
                if (!root.tabs)
                    return;
                if (event.angleDelta.y < 0)
                    root.tabs.selectNext();
                else
                    root.tabs.selectPrevious();
            }
        }

        delegate: GlassTab {
            id: tabDelegate

            required property int index
            required property var tab

            width: root.tabWidth
            height: list.height
            tabData: tabDelegate.tab
            active: root.tabs && root.tabs.currentIndex === tabDelegate.index

            onClicked: root.tabs.currentIndex = tabDelegate.index
            onCloseRequested: root.tabs.closeTab(tabDelegate.index)

            // Middle-click closes, as everywhere else.
            TapHandler {
                acceptedButtons: Qt.MiddleButton
                onTapped: root.tabs.closeTab(tabDelegate.index)
            }

            DragHandler {
                id: dragHandler
                target: null
                xAxis.enabled: true
                yAxis.enabled: false

                onActiveChanged: {
                    if (active)
                        list.interactive = false;
                    else
                        list.interactive = true;
                }

                onCentroidChanged: {
                    if (!active || !root.tabs)
                        return;
                    const point = tabDelegate.mapToItem(list, dragHandler.centroid.position.x,
                                                        tabDelegate.height / 2);
                    const target = list.indexAt(point.x + list.contentX, point.y);
                    if (target >= 0 && target !== tabDelegate.index)
                        root.tabs.moveTab(tabDelegate.index, target);
                }
            }
        }

        add: Transition {
            NumberAnimation {
                properties: "scale"
                from: 0.85
                to: 1.0
                duration: Theme.quick
                easing.type: Theme.easing
            }
            NumberAnimation {
                properties: "opacity"
                from: 0
                to: 1
                duration: Theme.quick
            }
        }

        remove: Transition {
            NumberAnimation {
                properties: "opacity"
                to: 0
                duration: Theme.instant
            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Theme.quick
                easing.type: Theme.easing
            }
        }

        move: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: Theme.quick
                easing.type: Theme.easing
            }
        }
    }

    GlassButton {
        id: newTabButton

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 30
        height: 30
        flat: true
        glyph: "+"
        tooltip: qsTr("New tab (Ctrl+T)")
        onClicked: root.tabs.addTab()
    }
}
