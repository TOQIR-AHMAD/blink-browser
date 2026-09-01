# The QML side of the browser.
#
# Four modules, one per directory, so each type's module URI matches where the
# file lives:
#
#   PrivacyBrowser.Ui             the window shell (this directory)
#   PrivacyBrowser.Ui.Theme       design tokens, as singletons
#   PrivacyBrowser.Ui.Components  the reusable glass components
#   PrivacyBrowser.Ui.Pages       the full-page views
add_subdirectory(theme)
add_subdirectory(components)
add_subdirectory(pages)
add_subdirectory(web)

qt_add_library(pb_ui STATIC)

qt_add_qml_module(pb_ui
    URI PrivacyBrowser.Ui
    VERSION 1.0
    QML_FILES
        BrowserWindow.qml
        Main.qml
    RESOURCES
        assets/icon.png
        assets/fonts/Ubuntu-R.ttf
        assets/fonts/Ubuntu-L.ttf
        assets/fonts/Ubuntu-M.ttf
        assets/fonts/Ubuntu-B.ttf
        assets/fonts/UbuntuMono-R.ttf
        assets/fonts/UbuntuMono-B.ttf
        assets/fonts/LICENCE-Ubuntu-font.txt
)

target_link_libraries(pb_ui
    PRIVATE
        Qt6::Quick
        Qt6::QuickControls2
        pb_ui_componentsplugin
        pb_ui_pagesplugin
        pb_ui_themeplugin
        pb_ui_webplugin
)
