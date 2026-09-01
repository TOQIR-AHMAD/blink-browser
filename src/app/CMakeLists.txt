qt_add_executable(PrivacyBrowser
    main.cpp
)

target_compile_definitions(PrivacyBrowser PRIVATE
    PB_VERSION="${PROJECT_VERSION}"
)

# pb_ui is the QML module defined in ui/CMakeLists.txt; linking its plugin
# target makes the PrivacyBrowser.Ui module available to the engine.
target_link_libraries(PrivacyBrowser PRIVATE
    Qt6::Quick
    PrivacyBrowser::Utils
    pb_uiplugin
)

# WIN32_EXECUTABLE keeps a release build from opening a console window; the
# debug build keeps stderr visible for the privacy-safe log.
set_target_properties(PrivacyBrowser PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
    MACOSX_BUNDLE TRUE
)
