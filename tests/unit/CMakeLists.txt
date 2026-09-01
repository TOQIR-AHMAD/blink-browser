# Tests use plain executables and CTest, so the Qt-free core can be tested
# without pulling in a test framework dependency.
function(pb_add_unit_test name)
    add_executable(test_${name} test_${name}.cpp)
    target_link_libraries(test_${name} PRIVATE PrivacyBrowser::Core)
    add_test(NAME ${name} COMMAND test_${name})
endfunction()

pb_add_unit_test(logging)
pb_add_unit_test(text)
pb_add_unit_test(url_info)
pb_add_unit_test(chromium_flags)
pb_add_unit_test(user_agent)
pb_add_unit_test(session_paths)
pb_add_unit_test(blocking_stats)
