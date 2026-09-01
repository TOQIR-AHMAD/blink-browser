# Browser architecture tests: tabs, windows, session history.
qt_add_executable(tst_browser tst_browser.cpp)

target_link_libraries(tst_browser PRIVATE
    PrivacyBrowser::Browser
    Qt6::Test
)

# Console subsystem, so the test's output reaches the terminal and CTest.
set_target_properties(tst_browser PROPERTIES WIN32_EXECUTABLE FALSE)

add_test(NAME integration COMMAND tst_browser)
