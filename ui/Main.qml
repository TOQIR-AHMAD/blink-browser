// Phase 1: proves the Chromium integration works - one window, one web view,
// one place to type an address. The tab strip, glass chrome and privacy UI
// arrive in the phases that follow, at which point this file becomes the
// window shell only.

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

Window {
    id: root

    width: 1200
    height: 800
    minimumWidth: 480
    minimumHeight: 360
    visible: true
    title: webView.title !== "" ? webView.title : qsTr("Privacy Browser")
    color: "#101014"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 8
            spacing: 8

            Button {
                text: "◀"
                enabled: webView.canGoBack
                onClicked: webView.goBack()
            }
            Button {
                text: "▶"
                enabled: webView.canGoForward
                onClicked: webView.goForward()
            }
            Button {
                text: webView.loading ? "✕" : "↻"
                onClicked: webView.loading ? webView.stop() : webView.reload()
            }

            TextField {
                id: addressField

                Layout.fillWidth: true
                placeholderText: qsTr("Enter an address")
                selectByMouse: true
                onAccepted: webView.url = text

                Connections {
                    target: webView
                    function onUrlChanged() {
                        if (!addressField.activeFocus)
                            addressField.text = webView.url.toString();
                    }
                }
            }
        }

        WebEngineView {
            id: webView

            Layout.fillWidth: true
            Layout.fillHeight: true

            // The off-the-record profile built in C++. Binding it here is what
            // keeps the browser off Qt WebEngine's disk-backed default profile.
            profile: App.profile
            url: "about:blank"
        }
    }
}
