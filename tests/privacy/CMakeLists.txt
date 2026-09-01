# Privacy verification (PLAN.md §42).
#
# The source audit needs nothing but Python, so it runs in every configuration,
# including the Qt-free one. The runtime checks need the real application.
find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_Interpreter_FOUND)
    add_test(NAME privacy_source_audit
             COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/audit_sources.py"
                     "${CMAKE_SOURCE_DIR}")
else()
    message(STATUS "Python 3 not found; skipping the source privacy audit")
endif()

if(NOT PB_BUILD_UI)
    return()
endif()

qt_add_executable(tst_persistence tst_persistence.cpp)

target_link_libraries(tst_persistence PRIVATE
    PrivacyBrowser::Browser
    Qt6::Test
)

add_test(NAME privacy_persistence COMMAND tst_persistence)
