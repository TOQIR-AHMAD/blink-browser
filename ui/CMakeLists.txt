# All QML lives in one module, PrivacyBrowser.Ui. Reusable glass components
# (components/), pages (pages/) and the theme singletons (theme/) are added to
# this module in later phases.
qt_add_library(pb_ui STATIC)

qt_add_qml_module(pb_ui
    URI PrivacyBrowser.Ui
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(pb_ui PRIVATE Qt6::Quick Qt6::QuickControls2)
