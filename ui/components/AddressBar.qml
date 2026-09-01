// The address bar.
//
// Two privacy-relevant details live here:
//
// - Completions come from this session's own history and nothing else. There
//   is no remote autocomplete service, and with search suggestions off (the
//   default) nothing is sent anywhere until you press Enter (PLAN.md §10).
// - What you type is classified locally into "address" or "search"; a
//   javascript: or data: URL is never navigated to from here.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Theme

FocusScope {
    id: root

    required property var controller // pb::browser::WindowController
    property var tab: null
    property bool editing: field.inputItem.activeFocus

    signal navigationRequested(url target)

    implicitHeight: 38

    function focusAddress() {
        field.focusInput();
        field.selectAllText();
    }

    function resetToPageUrl() {
        field.text = root.tab && root.tab.url.toString() !== "about:blank"
                ? root.tab.url.toString() : "";
        suggestions.close();
    }

    function commit(text) {
        const target = root.controller.resolveInput(text);
        if (target && target.toString() !== "") {
            suggestions.close();
            root.navigationRequested(target);
            field.inputItem.focus = false;
        }
    }

    Connections {
        target: root.tab
        enabled: root.tab !== null
        function onUrlChanged() {
            if (!root.editing)
                root.resetToPageUrl();
        }
    }

    onTabChanged: resetToPageUrl()

    GlassTextField {
        id: field

        anchors.fill: parent
        placeholderText: qsTr("Search or enter an address")
        selectAllOnFocus: true
        focus: true

        leading: [
            SecurityIndicator {
                anchors.verticalCenter: parent.verticalCenter
                level: root.tab ? root.tab.securityLevel : 0
                issue: root.tab ? root.tab.certificateIssue : ""
            }
        ]

        trailing: [
            GlassButton {
                anchors.verticalCenter: parent.verticalCenter
                width: 26
                height: 26
                flat: true
                glyph: root.tab && root.tab.loading ? "✕" : "↻"
                tooltip: root.tab && root.tab.loading ? qsTr("Stop") : qsTr("Reload (Ctrl+R)")
                visible: root.tab !== null
                onClicked: {
                    if (!root.tab)
                        return;
                    if (root.tab.loading)
                        root.tab.stop();
                    else
                        root.tab.reload();
                }
            }
        ]

        onAccepted: function (text) {
            root.commit(text);
        }
        onEscaped: {
            root.resetToPageUrl();
            field.inputItem.focus = false;
        }
        onTextChanged: {
            if (root.editing)
                suggestions.refresh(field.text);
        }
        onEditingStarted: suggestions.refresh(field.text)

        Keys.onDownPressed: suggestions.moveSelection(1)
        Keys.onUpPressed: suggestions.moveSelection(-1)
        Keys.onReturnPressed: function (event) {
            const chosen = suggestions.selectedUrl();
            if (chosen !== "") {
                suggestions.close();
                root.navigationRequested(chosen);
                field.inputItem.focus = false;
            } else {
                root.commit(field.text);
            }
            event.accepted = true;
        }
    }

    SuggestionList {
        id: suggestions

        anchors.top: field.bottom
        anchors.topMargin: Spacing.small
        anchors.left: field.left
        anchors.right: field.right
        controller: root.controller
        visible: count > 0 && root.editing

        onActivated: function (target) {
            suggestions.close();
            root.navigationRequested(target);
            field.inputItem.focus = false;
        }
    }
}
