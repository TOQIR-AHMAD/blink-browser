// One browser window: the chrome, the web views, and the overlays.
//
// The web views are created one per tab and kept alive in the background, so
// switching tabs does not reload anything. Every view is bound to the single
// off-the-record profile created in C++ - there is no code path here that can
// reach Qt WebEngine's disk-backed default profile.

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Pages
import PrivacyBrowser.Ui.Theme
import PrivacyBrowser.Ui.Web

Window {
    id: browserWindow

    required property var windowController

    readonly property var tabs: windowController ? windowController.tabs : null
    readonly property var currentTab: tabs ? tabs.currentTab : null
    readonly property bool privateMode: windowController ? windowController.privateMode : false

    // "", "settings", "dashboard", "downloads", "welcome"
    property string overlayPage: App.firstRun ? "welcome" : ""

    width: 1280
    height: 820
    minimumWidth: 560
    minimumHeight: 400
    visible: true
    color: Colors.background
    title: {
        const base = currentTab && currentTab.displayTitle !== ""
                   ? currentTab.displayTitle : qsTr("Privacy Browser");
        return privateMode ? qsTr("%1 — Private").arg(base) : base;
    }

    onClosing: function (close) {
        windowController.requestClose();
    }

    Component.onCompleted: {
        if (tabs && tabs.count === 0)
            tabs.addTab();
    }

    // -- Background ---------------------------------------------------------

    Item {
        id: windowContent

        anchors.fill: parent

        Rectangle {
            anchors.fill: parent
            color: Colors.background
        }

        // The aurora: three soft washes of colour that give the glass
        // something to be glass over.
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: browserWindow.privateMode ? Colors.auroraTwo
                                                                        : Colors.auroraOne }
                GradientStop { position: 0.45; color: Colors.background }
                GradientStop { position: 1.0; color: Colors.auroraThree }
            }
            opacity: Theme.glassEnabled ? 0.75 : 0.35

            Behavior on opacity {
                NumberAnimation { duration: Theme.slow }
            }
        }

        // -- Chrome ---------------------------------------------------------

        GlassSurface {
            id: chrome

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: Spacing.small
            height: tabStrip.height + navigation.height + Spacing.small * 2
            cornerRadius: Radius.xlarge
            elevated: true

            TabBar {
                id: tabStrip

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: Spacing.small
                tabs: browserWindow.tabs
            }

            NavigationBar {
                id: navigation

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: tabStrip.bottom
                anchors.leftMargin: Spacing.medium
                anchors.rightMargin: Spacing.medium
                controller: browserWindow.windowController
                tab: browserWindow.currentTab
                blockedCount: App.privacy.totalBlocked

                onPrivacyRequested: browserWindow.overlayPage = browserWindow.overlayPage === "dashboard"
                                                       ? "" : "dashboard"
                onMenuRequested: function (anchorItem) {
                    mainMenu.popup(anchorItem, 0, anchorItem.height + Spacing.tiny);
                }
                onNavigationRequested: function (target) {
                    browserWindow.overlayPage = "";
                    if (browserWindow.currentTab)
                        browserWindow.currentTab.navigate(target);
                }
            }
        }

        // -- Web content ----------------------------------------------------

        Item {
            id: contentArea

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: chrome.bottom
            anchors.bottom: parent.bottom
            anchors.margins: Spacing.small
            anchors.topMargin: Spacing.small

            Rectangle {
                anchors.fill: parent
                radius: Radius.large
                color: Colors.contentBackground
                border.width: 1
                border.color: Colors.glassBorder
                clip: true

                Repeater {
                    id: views

                    model: browserWindow.tabs

                    // WebArea is either a real WebEngineView or, in a build
                    // without Chromium, a panel that says so. Everything that
                    // needs an answer from the user is reported up to this
                    // window, which owns the overlays that ask.
                    delegate: WebArea {
                        id: view

                        required property int index
                        required property var tab

                        anchors.fill: parent
                        visible: browserWindow.tabs.currentIndex === view.index
                        focus: visible
                        tabData: view.tab
                        controller: browserWindow.windowController

                        onLoadSucceeded: function (pageUrl, pageTitle) {
                            view.tab.securityLevel = browserWindow.securityLevelFor(pageUrl,
                                                                                    view.tab);
                            browserWindow.windowController.recordVisit(pageUrl, pageTitle);
                            errorOverlay.visible = false;
                        }

                        onLoadFailed: function (failedUrl, errorCode, errorText, wasHttps) {
                            errorOverlay.host = browserWindow.hostOf(failedUrl);
                            errorOverlay.errorCode = errorCode;
                            errorOverlay.errorText = errorText;
                            // The fallback is offered only when an https attempt
                            // actually failed and we were the ones who upgraded.
                            errorOverlay.httpsFallbackOffered = wasHttps && App.settings.httpsFirst;
                            errorOverlay.failedUrl = failedUrl;
                            errorOverlay.visible = true;
                        }

                        onCertificateProblem: function (error) {
                            certificateWarning.pending = error;
                            certificateWarning.host = browserWindow.hostOf(error.url);
                            certificateWarning.description = error.description;
                            certificateWarning.overridable = error.overridable;
                            certificateWarning.visible = true;
                        }

                        onPermissionAsked: function (permission) {
                            const feature = App.permissions.featureFromWebEngine(
                                permission.permissionType);
                            const origin = permission.origin.toString();
                            const decision = App.permissions.decisionFor(origin, feature);

                            if (decision === 1) {
                                permission.grant();
                                App.permissions.recordOutcome(1);
                                return;
                            }
                            if (decision === 2) {
                                permission.deny();
                                App.permissions.recordOutcome(2);
                                return;
                            }

                            permissionPrompt.pending = permission;
                            permissionPrompt.ask(origin, feature,
                                                 App.permissions.featureName(feature),
                                                 App.permissions.featureDescription(feature));
                        }

                        onWindowRequested: function (target, separateWindow, background) {
                            if (separateWindow) {
                                browserWindow.windowController.openInNewWindow(
                                    target, browserWindow.privateMode);
                            } else {
                                browserWindow.tabs.addTab(target, background);
                            }
                        }

                        onFullScreenToggled: function (on) {
                            if (on)
                                browserWindow.showFullScreen();
                            else
                                browserWindow.showNormal();
                        }
                    }
                }

                // The new-tab page sits above the (blank) view of a fresh tab.
                NewTab {
                    id: newTabPage

                    anchors.fill: parent
                    visible: browserWindow.currentTab
                             && (browserWindow.currentTab.url.toString() === ""
                                 || browserWindow.currentTab.url.toString() === "about:blank")
                             && browserWindow.overlayPage === ""
                    controller: browserWindow.windowController
                    privacy: App.privacy

                    onNavigationRequested: function (target) {
                        if (browserWindow.currentTab)
                            browserWindow.currentTab.navigate(target);
                    }
                }

                ErrorOverlay {
                    id: errorOverlay

                    property url failedUrl: ""

                    anchors.fill: parent
                    visible: false

                    onRetryRequested: {
                        visible = false;
                        if (browserWindow.currentTab)
                            browserWindow.currentTab.navigate(failedUrl);
                    }
                    onHttpFallbackRequested: {
                        // Remember that this host has no working https, then
                        // load it over http with the address bar saying so.
                        App.privacy.recordHttpsFailure(host);
                        const downgraded = failedUrl.toString().replace("https://", "http://");
                        visible = false;
                        if (browserWindow.currentTab)
                            browserWindow.currentTab.navigate(downgraded);
                    }
                }

                CrashOverlay {
                    id: crashOverlay

                    anchors.fill: parent
                    visible: browserWindow.currentTab !== null
                             && browserWindow.currentTab.crashed
                    report: App.technicalReport(qsTr("A tab's render process exited."))

                    onReloadRequested: {
                        if (browserWindow.currentTab) {
                            browserWindow.currentTab.crashed = false;
                            browserWindow.currentTab.reload();
                        }
                    }
                }

                CertificateWarning {
                    id: certificateWarning

                    property var pending: null

                    anchors.fill: parent
                    visible: false

                    onGoBackRequested: {
                        if (pending)
                            pending.rejectCertificate();
                        pending = null;
                        visible = false;
                        if (browserWindow.currentTab && browserWindow.currentTab.canGoBack)
                            browserWindow.currentTab.goBack();
                    }
                    onProceedRequested: {
                        if (pending)
                            pending.acceptCertificate();
                        pending = null;
                        visible = false;
                        if (browserWindow.currentTab)
                            browserWindow.currentTab.securityLevel = 4;
                    }
                }
            }
        }

        // -- Prompts and overlays -------------------------------------------

        PermissionPrompt {
            id: permissionPrompt

            property var pending: null

            anchors.top: chrome.bottom
            anchors.topMargin: Spacing.small
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - Spacing.xlarge * 2, 520)

            onAllowed: function (remember) {
                if (pending)
                    pending.grant();
                if (remember)
                    App.permissions.remember(origin, feature, 1);
                else
                    App.permissions.recordOutcome(1);
                pending = null;
            }
            onBlocked: function (remember) {
                if (pending)
                    pending.deny();
                if (remember)
                    App.permissions.remember(origin, feature, 2);
                else
                    App.permissions.recordOutcome(2);
                pending = null;
            }
        }

        Loader {
            id: overlayLoader

            anchors.fill: contentArea
            active: browserWindow.overlayPage !== ""
            visible: active
            sourceComponent: {
                switch (browserWindow.overlayPage) {
                case "settings":
                    return settingsComponent;
                case "dashboard":
                    return dashboardComponent;
                case "downloads":
                    return downloadsComponent;
                case "welcome":
                    return welcomeComponent;
                default:
                    return null;
                }
            }
        }
    }

    // -- Overlay components -------------------------------------------------

    Component {
        id: settingsComponent

        Rectangle {
            radius: Radius.large
            color: Colors.background
            border.width: 1
            border.color: Colors.glassBorder
            clip: true

            Settings {
                anchors.fill: parent
                app: App
                settings: App.settings
                privacy: App.privacy
                permissions: App.permissions
            }

            GlassButton {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: Spacing.medium
                glyph: "✕"
                flat: true
                width: 32
                height: 32
                tooltip: qsTr("Close settings")
                onClicked: browserWindow.overlayPage = ""
            }
        }
    }

    Component {
        id: dashboardComponent

        Rectangle {
            radius: Radius.large
            color: Colors.background
            border.width: 1
            border.color: Colors.glassBorder
            clip: true

            PrivacyDashboard {
                anchors.fill: parent
                privacy: App.privacy
                permissions: App.permissions
            }

            GlassButton {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: Spacing.medium
                glyph: "✕"
                flat: true
                width: 32
                height: 32
                tooltip: qsTr("Close")
                onClicked: browserWindow.overlayPage = ""
            }
        }
    }

    Component {
        id: downloadsComponent

        Rectangle {
            radius: Radius.large
            color: Colors.background
            border.width: 1
            border.color: Colors.glassBorder
            clip: true

            DownloadsPanel {
                anchors.centerIn: parent
                width: Math.min(parent.width - Spacing.page * 2, 620)
                downloads: App.downloads
                onSaveRequested: function (item) {
                    saveDialog.pendingItem = item;
                    saveDialog.currentFolder = Qt.resolvedUrl(
                        "file:///" + App.downloads.suggestedSaveDirectory());
                    saveDialog.open();
                }
            }

            GlassButton {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: Spacing.medium
                glyph: "✕"
                flat: true
                width: 32
                height: 32
                onClicked: browserWindow.overlayPage = ""
            }
        }
    }

    Component {
        id: welcomeComponent

        FirstRun {
            onDismissed: browserWindow.overlayPage = ""
        }
    }

    // -- Menu ---------------------------------------------------------------

    GlassMenu {
        id: mainMenu

        MenuItem {
            text: qsTr("New tab")
            onTriggered: browserWindow.tabs.addTab()
        }
        MenuItem {
            text: qsTr("New window")
            onTriggered: App.browser.createWindow(false)
        }
        MenuItem {
            text: qsTr("New private window")
            onTriggered: App.browser.createWindow(true)
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Downloads")
            onTriggered: browserWindow.overlayPage = "downloads"
        }
        MenuItem {
            text: qsTr("Privacy dashboard")
            onTriggered: browserWindow.overlayPage = "dashboard"
        }
        MenuItem {
            text: qsTr("Settings")
            onTriggered: browserWindow.overlayPage = "settings"
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Clear browsing data now")
            onTriggered: App.privacy.clearBrowsingDataNow()
        }
        MenuItem {
            text: qsTr("Copy page address")
            enabled: browserWindow.currentTab !== null
            onTriggered: browserWindow.copyCurrentUrl()
        }
        MenuSeparator {}
        MenuItem {
            text: qsTr("Close window")
            onTriggered: browserWindow.windowController.requestClose()
        }
    }

    FileDialog {
        id: saveDialog

        property var pendingItem: null

        fileMode: FileDialog.SaveFile
        onAccepted: {
            if (pendingItem)
                App.downloads.saveAs(pendingItem, selectedFile);
            pendingItem = null;
        }
        onRejected: pendingItem = null
    }

    // Somewhere to put the URL for "copy page address" without a clipboard
    // permission dance: an off-screen, read-only TextEdit.
    TextEdit {
        id: clipboardHelper
        visible: false
        readOnly: true
    }

    // -- Keyboard -----------------------------------------------------------

    Shortcut {
        sequences: [StandardKey.AddTab]
        onActivated: browserWindow.tabs.addTab()
    }
    Shortcut {
        sequences: [StandardKey.Close]
        onActivated: browserWindow.tabs.closeCurrentTab()
    }
    Shortcut {
        sequence: "Ctrl+Shift+T"
        onActivated: browserWindow.tabs.reopenClosedTab()
    }
    Shortcut {
        sequences: [StandardKey.Refresh]
        onActivated: if (browserWindow.currentTab) browserWindow.currentTab.reload()
    }
    Shortcut {
        sequence: "Ctrl+Shift+R"
        onActivated: if (browserWindow.currentTab) browserWindow.currentTab.reloadBypassingCache()
    }
    Shortcut {
        sequences: [StandardKey.Back]
        onActivated: if (browserWindow.currentTab) browserWindow.currentTab.goBack()
    }
    Shortcut {
        sequences: [StandardKey.Forward]
        onActivated: if (browserWindow.currentTab) browserWindow.currentTab.goForward()
    }
    Shortcut {
        sequence: "Ctrl+L"
        onActivated: {
            browserWindow.overlayPage = "";
            navigation.focusAddress();
        }
    }
    Shortcut {
        sequence: "Ctrl+Shift+P"
        onActivated: App.browser.createWindow(true)
    }
    Shortcut {
        sequence: "Ctrl+Shift+N"
        onActivated: App.browser.createWindow(false)
    }
    Shortcut {
        sequence: "Ctrl+J"
        onActivated: browserWindow.overlayPage = browserWindow.overlayPage === "downloads" ? "" : "downloads"
    }
    Shortcut {
        sequence: "Ctrl+,"
        onActivated: browserWindow.overlayPage = browserWindow.overlayPage === "settings" ? "" : "settings"
    }
    Shortcut {
        sequence: "Ctrl+Tab"
        onActivated: browserWindow.tabs.selectNext()
    }
    Shortcut {
        sequence: "Ctrl+Shift+Tab"
        onActivated: browserWindow.tabs.selectPrevious()
    }
    Shortcut {
        sequences: [StandardKey.ZoomIn, "Ctrl+="]
        onActivated: browserWindow.setZoom(browserWindow.zoomFactor + 0.1)
    }
    Shortcut {
        sequences: [StandardKey.ZoomOut]
        onActivated: browserWindow.setZoom(browserWindow.zoomFactor - 0.1)
    }
    Shortcut {
        sequence: "Ctrl+0"
        onActivated: browserWindow.setZoom(1.0)
    }
    Shortcut {
        sequence: "Escape"
        onActivated: {
            if (browserWindow.overlayPage !== "" && browserWindow.overlayPage !== "welcome")
                browserWindow.overlayPage = "";
            else if (browserWindow.currentTab && browserWindow.currentTab.loading)
                browserWindow.currentTab.stop();
        }
    }

    // -- Helpers ------------------------------------------------------------

    property real zoomFactor: 1.0

    function setZoom(factor) {
        zoomFactor = Math.max(0.25, Math.min(4.0, factor));
        for (let i = 0; i < views.count; ++i) {
            const item = views.itemAt(i);
            if (item)
                item.zoomFactor = zoomFactor;
        }
    }

    function hostOf(value) {
        const text = value.toString();
        const schemeEnd = text.indexOf("://");
        const rest = schemeEnd < 0 ? text : text.substring(schemeEnd + 3);

        let end = rest.length;
        const stops = ["/", "?", "#"];
        for (let i = 0; i < stops.length; ++i) {
            const at = rest.indexOf(stops[i]);
            if (at >= 0 && at < end)
                end = at;
        }

        const authority = rest.substring(0, end);
        const credentials = authority.lastIndexOf("@");
        return credentials < 0 ? authority : authority.substring(credentials + 1);
    }

    function securityLevelFor(value, tab) {
        const text = value.toString();
        if (tab && tab.securityLevel === 4)
            return 4; // a certificate the user accepted stays flagged
        if (text.startsWith("https://"))
            return 3;
        if (text.startsWith("http://"))
            return 2;
        if (text === "" || text === "about:blank")
            return 0;
        return 1;
    }

    function copyCurrentUrl() {
        if (!currentTab)
            return;
        clipboardHelper.text = currentTab.url.toString();
        clipboardHelper.selectAll();
        clipboardHelper.copy();
        clipboardHelper.text = "";
    }
}
