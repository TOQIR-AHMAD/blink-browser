// Settings > Privacy.
//
// The switches at the top are things the user can change. The list underneath
// is the set of things that are not settings at all because the browser does
// not implement them; it is generated from
// pb::settings::privacyGuarantees() so the page cannot drift from the code.

import QtQuick
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Column {
    id: root

    required property var settings
    required property var privacy

    spacing: Spacing.large

    GlassCard {
        width: parent.width
        title: qsTr("Blocking")

        GlassToggle {
            width: parent.width
            label: qsTr("Block trackers")
            description: qsTr("Stops known measurement and session-recording endpoints before "
                              + "the connection is made.")
            checked: root.settings.trackerBlocking
            onToggled: root.settings.trackerBlocking = checked
        }

        GlassToggle {
            width: parent.width
            label: qsTr("Block ads")
            description: qsTr("Blocks advertising exchanges and tag servers. Elements left "
                              + "behind on the page are not hidden - see the filter-list note.")
            checked: root.settings.adBlocking
            onToggled: root.settings.adBlocking = checked
        }

        SettingRow {
            width: parent.width
            label: qsTr("Cookies")
            description: qsTr("Cookies never survive the session, whatever this is set to.")
            control: GlassSelect {
                model: [qsTr("Allow all"), qsTr("Block third-party"), qsTr("Block all")]
                currentIndex: root.settings.cookiePolicy
                onActivated: root.settings.cookiePolicy = currentIndex
            }
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("Fingerprinting")
        subtitle: qsTr("Reduces how distinctive this browser looks. Strict trades some site "
                       + "compatibility for it.")

        SettingRow {
            width: parent.width
            label: qsTr("Protection level")
            description: {
                switch (root.settings.fingerprintProtection) {
                case 0:
                    return qsTr("Off: the browser reports everything a site asks for.");
                case 2:
                    return qsTr("Strict: also blocks canvas readback, masks the GPU name, "
                                + "smooths audio timing and sends a fixed language header. "
                                + "Some sites will misbehave.");
                default:
                    return qsTr("Standard: a plain Chrome user agent and no local IP address "
                                + "over WebRTC. Nothing else is changed.");
                }
            }
            control: GlassSelect {
                model: [qsTr("Off"), qsTr("Standard"), qsTr("Strict")]
                currentIndex: root.settings.fingerprintProtection
                onActivated: root.settings.fingerprintProtection = currentIndex
            }
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("Filter lists")
        subtitle: qsTr("%1 rules loaded").arg(root.privacy.filters.ruleCount)

        Repeater {
            model: root.privacy.filters.lists

            delegate: Item {
                required property var modelData

                width: parent.width
                height: 52

                Column {
                    anchors.left: parent.left
                    anchors.right: toggle.left
                    anchors.rightMargin: Spacing.medium
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        width: parent.width
                        text: modelData.title
                        color: Colors.text
                        font.family: Typography.family
                        font.pixelSize: Typography.body
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: qsTr("%1 · %2 rules · %3 skipped")
                              .arg(modelData.source).arg(modelData.rules).arg(modelData.skipped)
                        color: Colors.textFaint
                        font.family: Typography.family
                        font.pixelSize: Typography.caption
                        elide: Text.ElideMiddle
                    }
                }

                GlassToggle {
                    id: toggle
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    checked: modelData.enabled
                    onToggled: root.privacy.filters.setListEnabled(modelData.id, checked)
                }
            }
        }

        Text {
            width: parent.width
            text: qsTr("The built-in lists ship inside the browser and are never fetched. A "
                       + "list you add from a URL is fetched only when you ask for an update, "
                       + "and that request tells the list's host your IP address and the time "
                       + "- nothing about your browsing.")
            color: Colors.textFaint
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }

        Row {
            spacing: Spacing.small

            GlassButton {
                text: root.privacy.filters.updating ? qsTr("Updating…") : qsTr("Update lists now")
                enabled: !root.privacy.filters.updating
                onClicked: root.privacy.filters.updateRemoteLists()
            }
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("Not implemented")
        subtitle: qsTr("These are not switches that happen to be off. The code to do them is "
                       + "not in the browser.")

        Repeater {
            model: root.settings.privacyGuarantees()

            delegate: Row {
                required property var modelData
                width: parent.width

                Text {
                    width: parent.width * 0.6
                    text: modelData.label
                    color: Colors.text
                    font.family: Typography.family
                    font.pixelSize: Typography.body
                }

                Text {
                    text: modelData.state
                    color: Colors.secure
                    font.family: Typography.family
                    font.pixelSize: Typography.label
                    font.weight: Font.DemiBold
                }
            }
        }
    }
}
