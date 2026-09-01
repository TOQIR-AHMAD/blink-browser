# QML tests for the design system and the glass components.
find_package(Qt6 QUIET COMPONENTS QuickTest)

if(NOT TARGET Qt6::QuickTest)
    message(STATUS "Qt Quick Test not found; skipping the QML tests")
    return()
endif()

qt_add_executable(tst_ui tst_ui.cpp)

target_link_libraries(tst_ui PRIVATE
    Qt6::QuickTest
    pb_ui_componentsplugin
    pb_ui_themeplugin
)

add_test(NAME ui COMMAND tst_ui -input "${CMAKE_CURRENT_SOURCE_DIR}" -platform offscreen)
