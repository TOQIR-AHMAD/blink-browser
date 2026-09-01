// The web view, without a web engine.
//
// Used when the project is configured with -DPB_WEB_ENGINE=OFF, which builds
// the whole browser except Chromium. That configuration exists for a specific
// reason: Qt ships Qt WebEngine on Windows only for the MSVC build, so on a
// machine (or CI runner) with the MinGW Qt, this is the only way to compile
// and run the real interface, with the real C++ models behind it.
//
// It implements exactly the surface BrowserWindow.qml uses, and does nothing.
// It never emits the signals that would ask the user something, because
// nothing here can load a page. It is not a fake browser: it says on its face
// that there is no engine in this build.

import QtQuick
import PrivacyBrowser.Ui.Theme

Item {
    id: view

    property var tabData: null
    property var controller: null

    // Present so the window can drive zoom uniformly; nothing renders here.
    property real zoomFactor: 1.0

    signal loadSucceeded(url pageUrl, string pageTitle)
    signal loadFailed(url failedUrl, int errorCode, string errorText, bool wasHttps)
    signal certificateProblem(var error)
    signal permissionAsked(var permission)
    signal windowRequested(url target, bool separateWindow, bool background)
    signal fullScreenToggled(bool on)

    function reload() {}
    function reloadAndBypassCache() {}
    function stop() {}
    function goBack() {}
    function goForward() {}

    Rectangle {
        anchors.fill: parent
        color: Colors.contentBackground
    }

    Column {
        anchors.centerIn: parent
        width: Math.min(parent.width - Spacing.huge * 2, 520)
        spacing: Spacing.medium

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: "◍"
            font.pixelSize: 40
            color: Colors.textFaint
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Built without a web engine")
            color: Colors.text
            font.family: Typography.family
            font.pixelSize: Typography.title
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("This build was configured with -DPB_WEB_ENGINE=OFF, so there is no "
                       + "Chromium in it and no page can be loaded. Everything around this "
                       + "panel - tabs, the address bar, settings, the privacy dashboard - is "
                       + "the real interface running on the real models.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.body
            wrapMode: Text.WordWrap
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: view.tabData && view.tabData.url.toString() !== "" && view.tabData.url.toString() !== "about:blank"
                  ? qsTr("Requested: %1").arg(view.tabData.url)
                  : ""
            visible: text !== ""
            color: Colors.textFaint
            font.family: Typography.monoFamily
            font.pixelSize: Typography.caption
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("A build with Chromium needs Qt WebEngine, which Qt provides only for "
                       + "the MSVC toolchain on Windows. See docs/build-windows.md.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }

    // The address bar still needs somewhere to send a navigation, and the tab
    // still records what was asked for, so the rest of the browser behaves
    // exactly as it would with an engine.
    Connections {
        target: view.tabData
        enabled: view.tabData !== null

        function onNavigationRequested(target) {
            view.tabData.url = target;
            view.tabData.loading = false;
            // Nothing was fetched, so nothing is known about the connection.
            // Drawing a padlock here would be a lie.
            view.tabData.securityLevel = 0;
        }
    }
}
