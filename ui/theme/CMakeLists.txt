# The design tokens, as a QML module of singletons.
#
# Each directory under ui/ is its own module so the URI mirrors the layout and
# the types resolve without import-path guesswork.
set(pb_theme_singletons
    Colors.qml
    Radius.qml
    Shadows.qml
    Spacing.qml
    Theme.qml
    Typography.qml
)
set_source_files_properties(${pb_theme_singletons} PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_library(pb_ui_theme STATIC)

qt_add_qml_module(pb_ui_theme
    URI PrivacyBrowser.Ui.Theme
    VERSION 1.0
    QML_FILES ${pb_theme_singletons}
)

target_link_libraries(pb_ui_theme PRIVATE Qt6::Quick)
