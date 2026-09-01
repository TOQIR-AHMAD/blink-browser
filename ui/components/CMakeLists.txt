# The reusable glass components (PLAN.md §34).
qt_add_library(pb_ui_components STATIC)

qt_add_qml_module(pb_ui_components
    URI PrivacyBrowser.Ui.Components
    VERSION 1.0
    QML_FILES
        AddressBar.qml
        CertificateWarning.qml
        CrashOverlay.qml
        DownloadsPanel.qml
        ErrorOverlay.qml
        GlassButton.qml
        GlassCard.qml
        GlassDialog.qml
        GlassMenu.qml
        GlassSelect.qml
        GlassSlider.qml
        GlassSurface.qml
        GlassTab.qml
        GlassTextField.qml
        GlassToggle.qml
        NavigationBar.qml
        PermissionPrompt.qml
        PrivacyBadge.qml
        SecurityIndicator.qml
        SettingRow.qml
        StatTile.qml
        SuggestionList.qml
        TabBar.qml
)

target_link_libraries(pb_ui_components
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        pb_ui_themeplugin
)
