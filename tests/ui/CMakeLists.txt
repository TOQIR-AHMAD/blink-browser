# The QML side of the test suite.
#
# The binding check is a plain script and runs in every configuration - it is
# the only automated check on the C++/QML boundary that does not need Qt.
find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_Interpreter_FOUND)
    add_test(NAME qml_bindings
             COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/check_qml_bindings.py"
                     "${CMAKE_SOURCE_DIR}")
else()
    message(STATUS "Python 3 not found; skipping the QML binding check")
endif()

if(NOT PB_BUILD_UI)
    return()
endif()

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
