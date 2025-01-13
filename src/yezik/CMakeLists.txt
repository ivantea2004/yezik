
#[[
    Each module X has
        X.h file with interface
        X.c file with implementation
        X_test.c file with unit tests
]]

set(modules
    yezik
    lexer
    parser
)

add_executable(yezik
    main.c

    yezik.h
    yezik.c

    test.h
    test.c
)

target_include_directories(yezik PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

#[[

if (BUILD_TESTING)

    set(suite_dir ${CMAKE_CURRENT_BINARY_DIR})
    set(suite_header ${suite_dir}/yezik/yezik_test_suite.h)
    set(gen ${CMAKE_CURRENT_SOURCE_DIR}/yezik_test_suite_header_generator.cmake)
    
    get_target_property(sources yezik SOURCES)
    get_target_property(includes yezik INCLUDE_DIRECTORIES)

    add_executable(yezik_test
        ${sources}
        ${suite_header}
    )
    
    target_include_directories(yezik_test PRIVATE ${suite_dir} ${includes})
    target_compile_definitions(yezik_test PRIVATE "YEZIK_BUILD_TESTING=1")

    add_custom_command(
        COMMENT "Generating test suite header"
        OUTPUT ${suite_header}
        COMMAND cmake
            -DSUITE_HEADER=${suite_header}
            -DTEST_HEADER_INCLUDE=<yezik/test.h>
            -DTEST_DEF=YEZIK_TEST
            -DTEST_DECL=YEZIK_TEST_HEADER_DECLARE_TEST
            -DTEST_NAME=YEZIK_TEST_HEADER_GET_TEST_FN_NAME
            -DTEST_FN_TYPE=YezikTestCaseFn
            -DTEST_FN_ARR_NAME=yezik_test_cases
            -P ${gen}
            -- ${sources}
        DEPENDS ${sources} ${gen}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )

    set(test_include_name yezik_test_include.cmake)

    set(test_include ${CMAKE_CURRENT_BINARY_DIR}/${test_include_name})

    add_custom_command(
        COMMENT "Generating ctest files"
        TARGET yezik_test POST_BUILD
        BYPRODUCTS ${test_include}
        COMMAND yezik_test gen ${test_include} ${CMAKE_CURRENT_SOURCE_DIR}/../
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES ${test_include})

endif()
]]