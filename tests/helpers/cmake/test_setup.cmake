
#[[
    Discovers tests in given source files and
        calls add_test() to invoke individual test

    CONFIG_FILE is parsed
        and first 4 macro definitions are respectivly treated as
            TEST_DEF(...)
            TEST_DECL(...)
            TEST_FN_GET(...)
            TEST_FN_TYPE
        TEST_DEF(...) is considered a test definition, it must defined a function
        TEST_DECL(...) must declare the function TEST_DEF defined
        TEST_FN_GET(...) must give a name of defined function
        TEST_FN_TYPE must expand into function pointer type

    setup_tests_from_files(
        TEST_TARGET my_tests
        DEFINITIONS my_defs
        INCLUDE_DIRS my_includes
        LIBRARIES my_libs_to_link
        SOURCES files_with_tests
        CONFIG_FILE path/to/config.h
        WORKING_DIRECTORY dir/where/tests/are/executed
    )
]]
set(_TEST_SETUP_DIR ${CMAKE_CURRENT_LIST_DIR})
function(setup_tests_from_files)

    include(CMakeParseArguments)
    set(values "TEST_TARGET;CONFIG_FILE;WORKING_DIRECTORY")
    set(lists "DEFINITIONS;INCLUDE_DIRS;LIBRARIES;SOURCES")
    cmake_parse_arguments(
        ARG
        ""
        "${values}"
        "${lists}"
        ${ARGV}
    )
    
    foreach(i ${values})
        set(arg_i "ARG_${i}")
        
        if (NOT DEFINED ${arg_i})
            message(FATAL_ERROR "${i} is not provided")
        endif()
    endforeach()

    set(target ${ARG_TEST_TARGET})
    set(defs ${ARG_DEFINITIONS})
    set(includes ${ARG_INCLUDE_DIRS})
    set(libs ${ARG_LIBRARIES})
    set(sources ${ARG_SOURCES})
    set(config ${ARG_CONFIG_FILE})
    set(work_dir ${WORKING_DIRECTORY})

    get_filename_component(config_dir ${config} DIRECTORY)

    message("config dir = ${config_dir}")

    set(unique ${target}_suite)

    set(helpers_dir ${_TEST_SETUP_DIR}/../)
    set(suite_file helpers/test_suite.h)
    set(suite_dir ${CMAKE_CURRENT_BINARY_DIR}/${unique}/)
    set(discoverer ${helpers_dir}/cmake/test_discoverer.cmake)
    set(ctest_include ${CMAKE_BINARY_DIR}/${unique}_ctest_include.cmake)

    message("suite dir = ${suite_dir}")

    message("dis = ${discoverer}")
    # message("lolkek")

    #[[
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    ]]    

    add_executable(
        ${target}
        ${sources}
        ${suite_dir}/${suite_file}
        ${helpers_dir}/include/helpers/driver.h
        ${helpers_dir}/src/driver.c
    )

    target_include_directories(${target} PRIVATE ${includes} ${helpers_dir}/include ${config_dir} ${suite_dir})
    target_link_libraries(${target} PRIVATE ${libs})
    target_compile_definitions(${target} PRIVATE ${defs})

    get_target_property(dirs ${target} INCLUDE_DIRECTORIES)

    message("dirs = ${dirs}")

    file(MAKE_DIRECTORY ${suite_dir})

    add_custom_command(
        COMMENT "Collecting test suite for ${target}"
        OUTPUT ${suite_dir}/${suite_file}
        COMMAND cmake
            -DSUITE=${suite_file}
            -DCONFIG_FILE=${config}
            -P ${discoverer}
            -- ${sources}
        DEPENDS ${sources} ${discoverer} ${config}
        WORKING_DIRECTORY ${suite_dir}
        VERBATIM
    )

    add_custom_command(
        COMMENT "Generating ctest files for ${target}"
        TARGET ${target} POST_BUILD
        BYPRODUCTS ${ctest_include}
        COMMAND
            ${target}
            gen
            ${ctest_include}
            ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES ${test_include})

    add_test(NAME ${target} COMMAND ${target})

endfunction()
