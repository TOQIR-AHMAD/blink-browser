// Settings > Security.

import QtQuick
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Column {
    id: root

    required property var settings
    required property var app

    spacing: Spacing.large

    GlassCard {
        width: parent.width
        title: qsTr("Connections")

        GlassToggle {
            width: parent.width
            label: qsTr("Try HTTPS first")
            description: qsTr("A page typed or clicked as http:// is tried over https:// first. "
                              + "If the site genuinely has no HTTPS, it loads over http and the "
                              + "address bar says so.")
            checked: root.settings.httpsFirst
            onToggled: root.settings.httpsFirst = checked
        }

        Text {
            width: parent.width
            text: qsTr("Certificate validation is Chromium's and is never bypassed. A "
                       + "certificate error shows a full-page warning; when Chromium says the "
                       + "error may be overridden, continuing is a deliberate choice with the "
                       + "consequences spelled out.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }

    GlassCard {
        width: parent.width
        title: qsTr("Sandbox and isolation")
        subtitle: qsTr("Provided by Chromium, and not adjustable from here on purpose.")

        Repeater {
            model: [
                { name: qsTr("Process sandbox"), state: qsTr("On") },
                { name: qsTr("Site isolation"), state: qsTr("On") },
                { name: qsTr("Certificate validation"), state: qsTr("On") },
                { name: qsTr("Insecure content on HTTPS pages"), state: qsTr("Blocked") },
                { name: qsTr("Clipboard reading by pages"), state: qsTr("Blocked") },
                { name: qsTr("DNS prefetching"), state: qsTr("Off") },
                { name: qsTr("Hyperlink auditing (ping)"), state: qsTr("Off") },
                { name: qsTr("Background networking"), state: qsTr("Off") }
            ]

            delegate: Row {
                required property var modelData
                width: parent.width

                Text {
                    width: parent.width * 0.6
                    text: modelData.name
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

    GlassCard {
        width: parent.width
        title: qsTr("Crash reporting")
        subtitle: qsTr("There is no crash uploader in this browser.")

        Text {
            width: parent.width
            text: qsTr("If the browser crashes, nothing is sent anywhere. Chromium's crash "
                       + "database is written inside the session folder and deleted with it. "
                       + "You can export a report yourself if you want to send one to a "
                       + "developer - it may contain details about your machine, so read it "
                       + "before you do.")
            color: Colors.textSubtle
            font.family: Typography.family
            font.pixelSize: Typography.caption
            wrapMode: Text.WordWrap
        }
    }
}
