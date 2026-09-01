# The web view, in one of two versions.
#
# Both define the target pb_ui_web and the QML module PrivacyBrowser.Ui.Web
# with a type called WebArea, so BrowserWindow.qml is identical either way.
#
#   PB_WEB_ENGINE=ON   (default) engine/WebArea.qml - a real WebEngineView
#   PB_WEB_ENGINE=OFF            stub/WebArea.qml   - a panel that says so
#
# The OFF configuration exists because Qt ships Qt WebEngine on Windows only
# for the MSVC toolchain: with the MinGW Qt it is the only way to build and run
# the interface at all.
if(PB_WEB_ENGINE)
    add_subdirectory(engine)
else()
    add_subdirectory(stub)
endif()
