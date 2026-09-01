# ---------------------------------------------------------------------------
# pb_core - everything that does not need Qt.
#
# Blocking, URL handling, privacy policy, settings validation and session
# cleanup live here so they can be unit-tested without a Qt installation and
# without starting a browser. Anything that needs QObject, QML or Qt WebEngine
# belongs in pb_browser below.
# ---------------------------------------------------------------------------
add_library(pb_core STATIC
    utils/logging.cpp
    utils/logging.h
    utils/text.cpp
    utils/text.h
    chromium/core/chromium_flags.cpp
    chromium/core/chromium_flags.h
    chromium/core/user_agent.cpp
    chromium/core/user_agent.h
    network/core/url_info.cpp
    network/core/url_info.h
    privacy/core/blocking_stats.cpp
    privacy/core/blocking_stats.h
    privacy/core/privacy_types.h
    privacy/core/session_paths.cpp
    privacy/core/session_paths.h
)

target_include_directories(pb_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if(PB_DEV_LOGGING)
    target_compile_definitions(pb_core PUBLIC PB_DEV_LOGGING)
endif()

add_library(PrivacyBrowser::Core ALIAS pb_core)

if(NOT PB_BUILD_UI)
    return()
endif()

# ---------------------------------------------------------------------------
# pb_browser - the Qt layer: Chromium integration, tabs, windows, permissions,
# downloads and settings, all built on pb_core.
# ---------------------------------------------------------------------------
qt_add_library(pb_browser STATIC
    app/application.cpp
    app/application.h
    chromium/web_profile.cpp
    chromium/web_profile.h
)

target_compile_definitions(pb_browser PUBLIC PB_VERSION="${PROJECT_VERSION}")

target_link_libraries(pb_browser
    PUBLIC
        PrivacyBrowser::Core
        Qt6::Core
        Qt6::Gui
        Qt6::Quick
        Qt6::WebEngineQuick
)

add_library(PrivacyBrowser::Browser ALIAS pb_browser)

add_subdirectory(app)
