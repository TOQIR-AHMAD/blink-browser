// Settings.
//
// A category list on the left, one page at a time on the right. The Privacy,
// Security and About sections live in their own files; the rest are here
// because they are short.

import QtQuick
import QtQuick.Controls
import PrivacyBrowser.Ui.Components
import PrivacyBrowser.Ui.Theme

Item {
    id: root

    required property var app
    required property var settings
    required property var privacy
    required property var permissions

    property int section: 0

    readonly property var sections: [
        qsTr("General"), qsTr("Privacy"), qsTr("Security"), qsTr("Appearance"),
        qsTr("Search"), qsTr("Network"), qsTr("Permissions"), qsTr("Downloads"),
        qsTr("Advanced"), qsTr("About")
    ]

    Rectangle {
        anchors.fill: parent
        color: Colors.background
    }

    ListView {
        id: sidebar

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: Spacing.large
        width: 190
        spacing: Spacing.hair
        model: root.sections
        interactive: contentHeight > height

        delegate: AbstractButton {
            id: entry

            required property int index
            required property string modelData

            width: sidebar.width
            height: 36
            hoverEnabled: true
            onClicked: root.section = entry.index

            Accessible.role: Accessible.ListItem
            Accessible.name: entry.modelData

            background: Rectangle {
                radius: Radius.medium
                color: root.section === entry.index ? Colors.accentMuted
                                                    : (entry.hovered ? Colors.hover : "transparent")
            }

            contentItem: Text {
                leftPadding: Spacing.medium
                text: entry.modelData
                color: root.section === entry.index ? Colors.accent : Colors.text
                font.family: Typography.family
                font.pixelSize: Typography.body
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Flickable {
        id: pane

        anchors.left: sidebar.right
        anchors.leftMargin: Spacing.large
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: Spacing.large
        contentWidth: width
        contentHeight: content.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {}

        Loader {
            id: content

            width: Math.min(pane.width, 720)
            sourceComponent: {
                switch (root.section) {
                case 1:
                    return privacyPage;
                case 2:
                    return securityPage;
                case 3:
                    return appearancePage;
                case 4:
                    return searchPage;
                case 5:
                    return networkPage;
                case 6:
                    return permissionsPage;
                case 7:
                    return downloadsPage;
                case 8:
                    return advancedPage;
                case 9:
                    return aboutPage;
                default:
                    return generalPage;
                }
            }
        }
    }

    Component {
        id: generalPage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("Settings storage")
                subtitle: qsTr("Off by default, because a browser that writes nothing is easier "
                               + "to trust than one that promises it wrote the right thing.")

                GlassToggle {
                    width: parent.width
                    label: qsTr("Remember these settings on this device")
                    description: root.settings.rememberSettings
                                 ? qsTr("Stored at %1. Only settings - never history, cookies "
                                        + "or anything about browsing.").arg(root.settings.storagePath)
                                 : qsTr("Settings return to their defaults every launch.")
                    checked: root.settings.rememberSettings
                    onToggled: root.settings.rememberSettings = checked
                }

                GlassButton {
                    text: qsTr("Reset everything to defaults")
                    onClicked: root.settings.resetToDefaults()
                }
            }
        }
    }

    Component {
        id: privacyPage

        Privacy {
            settings: root.settings
            privacy: root.privacy
        }
    }

    Component {
        id: securityPage

        Security {
            settings: root.settings
            app: root.app
        }
    }

    Component {
        id: appearancePage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("Appearance")

                SettingRow {
                    width: parent.width
                    label: qsTr("Theme")
                    control: GlassSelect {
                        model: [qsTr("Follow the system"), qsTr("Light"), qsTr("Dark")]
                        currentIndex: root.settings.theme
                        onActivated: root.settings.theme = currentIndex
                    }
                }

                GlassToggle {
                    width: parent.width
                    label: qsTr("Shadows and blur")
                    description: qsTr("Soft shadows under menus and dialogs. Turning them off is "
                                      + "lighter on the GPU.")
                    checked: root.settings.glassEffects
                    onToggled: root.settings.glassEffects = checked
                }

                GlassToggle {
                    width: parent.width
                    label: qsTr("Reduce motion")
                    description: qsTr("Removes every animation in the interface.")
                    checked: root.settings.reducedMotion
                    onToggled: root.settings.reducedMotion = checked
                }

                GlassSlider {
                    width: parent.width
                    label: qsTr("Interface text size")
                    from: 0.8
                    to: 2.0
                    stepSize: 0.05
                    value: root.settings.textScale
                    valueText: Math.round(root.settings.textScale * 100) + "%"
                    onMoved: function (value) {
                        root.settings.textScale = value;
                    }
                }
            }
        }
    }

    Component {
        id: searchPage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("Search")
                subtitle: qsTr("Whatever you search for goes to the provider you pick here. "
                               + "It does not pass through anything of ours.")

                SettingRow {
                    width: parent.width
                    label: qsTr("Provider")
                    control: GlassSelect {
                        model: root.settings.searchEngines()
                        textRole: "name"
                        valueRole: "id"
                        currentIndex: {
                            const list = root.settings.searchEngines();
                            for (let i = 0; i < list.length; ++i) {
                                if (list[i].id === root.settings.searchEngineId)
                                    return i;
                            }
                            return 0;
                        }
                        onActivated: function (index) {
                            root.settings.searchEngineId = root.settings.searchEngines()[index].id;
                        }
                    }
                }

                GlassTextField {
                    width: parent.width
                    visible: root.settings.searchEngineId === "custom"
                    placeholderText: qsTr("https://example.com/search?q={searchTerms}")
                    text: root.settings.customSearchTemplate
                    onAccepted: function (text) {
                        root.settings.customSearchTemplate = text;
                    }
                }

                GlassToggle {
                    width: parent.width
                    label: qsTr("Search suggestions")
                    description: qsTr("Sends what you are typing to the search provider as you "
                                      + "type, before you press Enter. Off by default for that "
                                      + "reason.")
                    checked: root.settings.searchSuggestions
                    onToggled: root.settings.searchSuggestions = checked
                }
            }
        }
    }

    Component {
        id: networkPage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("DNS")
                subtitle: qsTr("Takes effect the next time the browser starts.")

                SettingRow {
                    width: parent.width
                    label: qsTr("Resolver")
                    description: root.settings.dnsMode === 1
                                 ? qsTr("Your DNS queries go to the provider below, which sees "
                                        + "every name you look up.")
                                 : qsTr("Your operating system's resolver decides, which usually "
                                        + "means your network or ISP sees every name you look up.")
                    control: GlassSelect {
                        model: [qsTr("System resolver"), qsTr("DNS over HTTPS")]
                        currentIndex: root.settings.dnsMode
                        onActivated: root.settings.dnsMode = currentIndex
                    }
                }

                GlassTextField {
                    width: parent.width
                    visible: root.settings.dnsMode === 1
                    placeholderText: qsTr("https://resolver.example/dns-query")
                    text: root.settings.dohTemplate
                    onAccepted: function (text) {
                        root.settings.dohTemplate = text;
                    }
                }
            }

            GlassCard {
                width: parent.width
                title: qsTr("Proxy")
                subtitle: qsTr("A proxy hides your address from the sites you visit and shows "
                               + "everything to the proxy instead. It is not anonymity.")

                SettingRow {
                    width: parent.width
                    label: qsTr("Mode")
                    control: GlassSelect {
                        model: [qsTr("System"), qsTr("None"), qsTr("HTTP"), qsTr("SOCKS5")]
                        currentIndex: root.settings.proxyMode
                        onActivated: root.settings.proxyMode = currentIndex
                    }
                }

                Row {
                    width: parent.width
                    spacing: Spacing.small
                    visible: root.settings.proxyMode >= 2

                    GlassTextField {
                        width: parent.width * 0.65
                        placeholderText: qsTr("Host")
                        text: root.settings.proxyHost
                        onAccepted: function (text) {
                            root.settings.proxyHost = text;
                        }
                    }

                    GlassTextField {
                        width: parent.width * 0.3
                        placeholderText: qsTr("Port")
                        text: root.settings.proxyPort > 0 ? root.settings.proxyPort : ""
                        onAccepted: function (text) {
                            root.settings.proxyPort = parseInt(text, 10) || 0;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: permissionsPage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("Permissions")
                subtitle: qsTr("Camera, microphone, location, notifications, clipboard, MIDI, "
                               + "USB, Bluetooth, screen sharing and file access all ask every "
                               + "time until you answer for a site, and every answer is "
                               + "forgotten when the browser closes.")

                Text {
                    width: parent.width
                    text: root.permissions.grantedCount + root.permissions.deniedCount > 0
                          ? qsTr("%1 allowed, %2 blocked in this session")
                            .arg(root.permissions.grantedCount).arg(root.permissions.deniedCount)
                          : qsTr("No site has asked for anything yet.")
                    color: Colors.textSubtle
                    font.family: Typography.family
                    font.pixelSize: Typography.body
                    wrapMode: Text.WordWrap
                }

                GlassButton {
                    text: qsTr("Forget all answers")
                    enabled: root.permissions.grantedCount + root.permissions.deniedCount > 0
                    onClicked: root.permissions.clear()
                }
            }
        }
    }

    Component {
        id: downloadsPage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("Downloads")
                subtitle: qsTr("A downloaded file is written to disk and stays there. That is "
                               + "the one thing this browser cannot keep in memory.")

                GlassToggle {
                    width: parent.width
                    label: qsTr("Ask where to save each file")
                    description: qsTr("The file is downloaded into the session folder first and "
                                      + "moved where you choose.")
                    checked: root.settings.askWhereToSave
                    onToggled: root.settings.askWhereToSave = checked
                }

                GlassTextField {
                    width: parent.width
                    visible: !root.settings.askWhereToSave
                    placeholderText: qsTr("Folder for downloads")
                    text: root.settings.downloadDirectory
                    onAccepted: function (text) {
                        root.settings.downloadDirectory = text;
                    }
                }
            }
        }
    }

    Component {
        id: advancedPage

        Column {
            spacing: Spacing.large

            GlassCard {
                width: parent.width
                title: qsTr("Filter list updates")

                GlassToggle {
                    width: parent.width
                    label: qsTr("Check added lists for updates at start-up")
                    description: qsTr("Off by default: an update is a request to the list's "
                                      + "host, and you should decide when to make it.")
                    checked: root.settings.checkFilterListUpdatesOnStart
                    onToggled: root.settings.checkFilterListUpdatesOnStart = checked
                }
            }

            GlassCard {
                width: parent.width
                title: qsTr("Network audit")
                subtitle: root.privacy.auditAvailable
                          ? qsTr("Records which hosts this browser connects to, by host and "
                                 + "count only, in memory.")
                          : qsTr("Not compiled into this build. Configure with "
                                 + "-DPB_NETWORK_AUDIT=ON to include it.")

                GlassToggle {
                    width: parent.width
                    enabled: root.privacy.auditAvailable
                    label: qsTr("Record outbound connections")
                    checked: root.privacy.auditEnabled
                    onToggled: root.privacy.auditEnabled = checked
                }

                Repeater {
                    model: root.privacy.auditEnabled ? root.privacy.auditRows() : []

                    delegate: Row {
                        required property var modelData
                        width: parent.width
                        spacing: Spacing.small

                        Text {
                            width: parent.width * 0.6
                            text: modelData.host
                            color: Colors.text
                            font.family: Typography.monoFamily
                            font.pixelSize: Typography.caption
                            elide: Text.ElideMiddle
                        }

                        Text {
                            text: qsTr("%1 requests, %2 blocked").arg(modelData.requests)
                                  .arg(modelData.blocked)
                            color: Colors.textSubtle
                            font.family: Typography.family
                            font.pixelSize: Typography.caption
                        }
                    }
                }

                GlassButton {
                    text: qsTr("Clear the audit")
                    visible: root.privacy.auditEnabled
                    onClicked: root.privacy.clearAudit()
                }
            }
        }
    }

    Component {
        id: aboutPage

        About {
            app: root.app
        }
    }
}
