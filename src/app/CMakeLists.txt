qt_add_executable(PrivacyBrowser
    main.cpp
)

# Each QML module contributes a plugin target; linking them is what makes the
# modules resolvable from the engine in a statically linked build.
target_link_libraries(PrivacyBrowser PRIVATE
    PrivacyBrowser::Browser
    pb_browserplugin
    pb_uiplugin
    pb_ui_themeplugin
    pb_ui_componentsplugin
    pb_ui_pagesplugin
    pb_ui_webplugin
)

# WIN32_EXECUTABLE keeps a release build from opening a console window; the
# debug build keeps stderr visible for the privacy-safe log.
set_target_properties(PrivacyBrowser PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
    MACOSX_BUNDLE TRUE
)

# Windows version information and the application icon.
if(WIN32)
    configure_file("${CMAKE_SOURCE_DIR}/packaging/windows/app.rc.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/app.rc" @ONLY)
    target_sources(PrivacyBrowser PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
endif()
