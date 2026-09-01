qt_add_executable(PrivacyBrowser
    main.cpp
)

# pb_uiplugin is the QML module declared in ui/CMakeLists.txt; linking it makes
# the PrivacyBrowser.Ui module available to the engine.
target_link_libraries(PrivacyBrowser PRIVATE
    PrivacyBrowser::Browser
    pb_uiplugin
)

# WIN32_EXECUTABLE keeps a release build from opening a console window; the
# debug build keeps stderr visible for the privacy-safe log.
set_target_properties(PrivacyBrowser PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
    MACOSX_BUNDLE TRUE
)

# Windows version information (and, from Phase 10, the application icon).
if(WIN32)
    configure_file("${CMAKE_SOURCE_DIR}/packaging/windows/app.rc.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/app.rc" @ONLY)
    target_sources(PrivacyBrowser PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
endif()
