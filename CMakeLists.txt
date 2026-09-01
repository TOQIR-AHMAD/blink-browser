cmake_minimum_required(VERSION 3.21)

project(PrivacyBrowser
    VERSION 0.1.0
    DESCRIPTION "Privacy-first desktop web browser"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
# PB_BUILD_UI=OFF builds only the platform-independent core libraries and their
# tests. It exists so the core can be compiled and tested on a machine (or CI
# runner) where Qt 6 is not installed. The shipping browser always needs Qt.
option(PB_BUILD_UI "Build the Qt/QML application shell" ON)
option(PB_BUILD_TESTS "Build the test suite" ON)

# Verbose, developer-only logging. Never enable for a release build: it allows
# log lines to keep the host part of a URL. See docs/architecture.md and
# src/utils/logging.h.
option(PB_DEV_LOGGING "Allow developer-detail (host-level) logging" OFF)

# Compiles in the outbound-connection recorder from PLAN.md section 43. It is a
# development tool: off in a release build, and off at runtime until the user
# turns it on in Settings > Advanced.
option(PB_NETWORK_AUDIT "Compile in the network audit recorder" OFF)

# Qt ships Qt WebEngine on Windows only for the MSVC toolchain. Turning this
# off builds the whole browser except Chromium - the real interface on the real
# models, with a placeholder where the page would be - which is the only build
# possible with the MinGW Qt. A browser needs it ON.
option(PB_WEB_ENGINE "Build with Qt WebEngine (Chromium)" ON)

# ---------------------------------------------------------------------------
# Global build settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(PB_BUILD_UI)
    # 6.8 is the floor because the browser uses the QWebEnginePermission API
    # introduced there, and because 6.8 is the current Qt LTS.
    set(PB_QT_COMPONENTS Core Gui Network Quick QuickControls2)
    if(PB_BUILD_TESTS)
        list(APPEND PB_QT_COMPONENTS Test)
    endif()
    if(PB_WEB_ENGINE)
        list(APPEND PB_QT_COMPONENTS WebEngineQuick)
    endif()
    find_package(Qt6 6.8 REQUIRED COMPONENTS ${PB_QT_COMPONENTS})
    qt_standard_project_setup(REQUIRES 6.8)
endif()

if(PB_BUILD_UI)
    # The QML module is declared before src/ so the application target can link
    # to it directly.
    add_subdirectory(ui)
endif()

add_subdirectory(src)

if(PB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "PrivacyBrowser ${PROJECT_VERSION}")
message(STATUS "  Qt/QML shell : ${PB_BUILD_UI}")
message(STATUS "  Tests        : ${PB_BUILD_TESTS}")
message(STATUS "  Dev logging  : ${PB_DEV_LOGGING}")
message(STATUS "  Network audit: ${PB_NETWORK_AUDIT}")
message(STATUS "  Web engine   : ${PB_WEB_ENGINE}")
