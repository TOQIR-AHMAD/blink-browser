# Browser architecture tests: tabs, windows, session history.
qt_add_executable(tst_browser tst_browser.cpp)

target_link_libraries(tst_browser PRIVATE
    PrivacyBrowser::Browser
    Qt6::Test
)

add_test(NAME integration COMMAND tst_browser)
