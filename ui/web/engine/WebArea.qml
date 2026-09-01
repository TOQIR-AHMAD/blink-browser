// The web view for one tab.
//
// This is the only file in the project that instantiates a WebEngineView, and
// it exists in two versions: this one, and a placeholder in ui/web/stub/ used
// when the project is configured with -DPB_WEB_ENGINE=OFF. Both register as
// the type WebArea in PrivacyBrowser.Ui.Web and expose the same properties and
// signals, so BrowserWindow.qml does not know which one it got.
//
// State flows up to the Tab object by assignment; commands come down as
// signals from it. Everything that needs a decision from the user (a
// certificate error, a permission request, a failed load) is reported to the
// window rather than handled here, so the browser has exactly one place where
// each of those is answered.

import QtQuick
import QtWebEngine

WebEngineView {
    id: view

    // Set by the window.
    property var tabData: null
    property var controller: null

    signal loadSucceeded(url pageUrl, string pageTitle)
    signal loadFailed(url failedUrl, int errorCode, string errorText, bool wasHttps)
    signal certificateProblem(var error)
    signal permissionAsked(var permission)
    signal windowRequested(url target, bool separateWindow, bool background)
    signal fullScreenToggled(bool on)

    // The one off-the-record profile, created in C++. There is no code path
    // here that can reach Qt WebEngine's disk-backed default profile.
    profile: App.profile

    url: view.tabData && view.tabData.initialUrl && view.tabData.initialUrl.toString() !== ""
         ? view.tabData.initialUrl : "about:blank"
    audioMuted: view.tabData ? view.tabData.muted : false

    settings.screenCaptureEnabled: false
    settings.javascriptCanAccessClipboard: false
    settings.allowRunningInsecureContent: false

    onTitleChanged: if (view.tabData) view.tabData.title = view.title
    onIconChanged: if (view.tabData) view.tabData.iconUrl = view.icon
    onLoadProgressChanged: if (view.tabData) view.tabData.loadProgress = view.loadProgress
    onCanGoBackChanged: if (view.tabData) view.tabData.canGoBack = view.canGoBack
    onCanGoForwardChanged: if (view.tabData) view.tabData.canGoForward = view.canGoForward
    onRecentlyAudibleChanged: if (view.tabData) view.tabData.audible = view.recentlyAudible
    onUrlChanged: if (view.tabData) view.tabData.url = view.url

    onLoadingChanged: function (loadRequest) {
        if (view.tabData)
            view.tabData.loading = loadRequest.status === WebEngineView.LoadStartedStatus;

        switch (loadRequest.status) {
        case WebEngineView.LoadStartedStatus:
            if (view.tabData)
                view.tabData.crashed = false;
            break;
        case WebEngineView.LoadSucceededStatus:
            view.loadSucceeded(view.url, view.title);
            break;
        case WebEngineView.LoadFailedStatus:
            view.loadFailed(loadRequest.url, loadRequest.errorCode, loadRequest.errorString,
                            loadRequest.url.toString().startsWith("https://"));
            break;
        default:
            break;
        }
    }

    onRenderProcessTerminated: function (terminationStatus, exitCode) {
        if (!view.tabData)
            return;
        view.tabData.crashed = true;
        view.tabData.loading = false;
    }

    onCertificateError: function (error) {
        // Deferring keeps the page unloaded while the user decides.
        error.defer();
        view.certificateProblem(error);
    }

    onPermissionRequested: function (permission) {
        view.permissionAsked(permission);
    }

    onNewWindowRequested: function (request) {
        view.windowRequested(request.requestedUrl,
                             request.destination === WebEngineNewWindowRequest.InNewWindow,
                             request.userInitiated === false);
    }

    onFullScreenRequested: function (request) {
        request.accept();
        view.fullScreenToggled(request.toggleOn);
    }

    Connections {
        target: view.tabData
        enabled: view.tabData !== null

        function onNavigationRequested(target) {
            view.url = target;
        }
        function onReloadRequested(bypassCache) {
            if (bypassCache)
                view.reloadAndBypassCache();
            else
                view.reload();
        }
        function onStopRequested() {
            view.stop();
        }
        function onBackRequested() {
            view.goBack();
        }
        function onForwardRequested() {
            view.goForward();
        }
    }
}
