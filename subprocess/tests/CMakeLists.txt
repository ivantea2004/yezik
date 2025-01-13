

# Testing if exit is handled properly
add_executable(exit_test exit_test.c)
target_link_libraries(exit_test PRIVATE subprocess::subprocess)
add_test(NAME Subprocess_Exit COMMAND exit_test)

# Testing if errors are handled properly
add_executable(error_test error_test.c)
target_link_libraries(error_test PRIVATE subprocess::subprocess)

add_test(NAME Subprocess_Error COMMAND error_test)
