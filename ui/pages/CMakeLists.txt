qt_add_library(pb_ui_pages STATIC)

qt_add_qml_module(pb_ui_pages
    URI PrivacyBrowser.Ui.Pages
    VERSION 1.0
    QML_FILES
        About.qml
        FirstRun.qml
        NewTab.qml
        Privacy.qml
        PrivacyDashboard.qml
        Security.qml
        Settings.qml
)

target_link_libraries(pb_ui_pages
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        pb_ui_componentsplugin
        pb_ui_themeplugin
)
