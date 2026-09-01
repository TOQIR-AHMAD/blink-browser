# Tests use plain executables and CTest, so the Qt-free core can be tested
# without pulling in a test framework dependency.
add_executable(test_logging test_logging.cpp)
target_link_libraries(test_logging PRIVATE PrivacyBrowser::Utils)
add_test(NAME logging COMMAND test_logging)
