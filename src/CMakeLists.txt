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
    browser/core/session_history.cpp
    browser/core/session_history.h
    chromium/core/chromium_flags.cpp
    chromium/core/chromium_flags.h
    chromium/core/user_agent.cpp
    chromium/core/user_agent.h
    network/core/filter_engine.cpp
    network/core/filter_engine.h
    network/core/filter_rule.cpp
    network/core/filter_rule.h
    network/core/https_upgrade.cpp
    network/core/https_upgrade.h
    network/core/network_audit.cpp
    network/core/network_audit.h
    network/core/url_info.cpp
    network/core/url_info.h
    permissions/core/permission_policy.cpp
    permissions/core/permission_policy.h
    privacy/core/blocking_stats.cpp
    privacy/core/blocking_stats.h
    privacy/core/cleanup_sequence.cpp
    privacy/core/cleanup_sequence.h
    privacy/core/privacy_types.h
    privacy/core/session_paths.cpp
    privacy/core/session_paths.h
    settings/core/browser_settings.cpp
    settings/core/browser_settings.h
    settings/core/omnibox_input.cpp
    settings/core/omnibox_input.h
    settings/core/search_engines.cpp
    settings/core/search_engines.h
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
# downloads and settings, all built on pb_core. It is a QML module so the QML
# side can name the C++ types and their enums.
# ---------------------------------------------------------------------------
qt_add_library(pb_browser STATIC
    app/application.cpp
    app/application.h
    browser/browser_controller.cpp
    browser/browser_controller.h
    browser/window_controller.cpp
    browser/window_controller.h
    chromium/web_profile.cpp
    chromium/web_profile.h
    downloads/download_manager.cpp
    downloads/download_manager.h
    network/filter_service.cpp
    network/filter_service.h
    network/request_interceptor.cpp
    network/request_interceptor.h
    permissions/permission_manager.cpp
    permissions/permission_manager.h
    privacy/privacy_manager.cpp
    privacy/privacy_manager.h
    settings/settings_controller.cpp
    settings/settings_controller.h
    tabs/tab.cpp
    tabs/tab.h
    tabs/tab_model.cpp
    tabs/tab_model.h
)

qt_add_qml_module(pb_browser
    URI PrivacyBrowser.Core
    VERSION 1.0
)

target_compile_definitions(pb_browser PUBLIC PB_VERSION="${PROJECT_VERSION}")

if(PB_NETWORK_AUDIT)
    target_compile_definitions(pb_browser PUBLIC PB_NETWORK_AUDIT)
endif()

# Filter lists and the fingerprint-protection script are compiled into the
# binary: a default install fetches nothing in order to be able to block.
qt_add_resources(pb_browser "pb_resources"
    PREFIX "/"
    BASE "${CMAKE_SOURCE_DIR}/resources"
    FILES
        "${CMAKE_SOURCE_DIR}/resources/filters/ads.txt"
        "${CMAKE_SOURCE_DIR}/resources/filters/trackers.txt"
        "${CMAKE_SOURCE_DIR}/resources/scripts/fingerprint_protection.js"
)

target_link_libraries(pb_browser
    PUBLIC
        PrivacyBrowser::Core
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Quick
        Qt6::WebEngineQuick
)

add_library(PrivacyBrowser::Browser ALIAS pb_browser)

add_subdirectory(app)
