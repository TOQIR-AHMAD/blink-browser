qt_add_library(pb_ui_web STATIC)

qt_add_qml_module(pb_ui_web
    URI PrivacyBrowser.Ui.Web
    VERSION 1.0
    QML_FILES
        WebArea.qml
)

target_link_libraries(pb_ui_web PRIVATE Qt6::Quick Qt6::WebEngineQuick)
