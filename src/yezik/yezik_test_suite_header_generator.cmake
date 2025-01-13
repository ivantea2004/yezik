#[[

    This file is a cmake script to generate a test suite.

    Intended to be called like:
        cmake
            -DSUITE_HEADER=path/to/suite/header.h
            -DTEST_HEADER_INCLUDE=<what/to/include.h>
            -DTEST_DEF=macro_to_define_test_case
            -DTEST_DECL=macro_to_declare_test_case_function
            -DTEST_NAME=macro_to_get_test_case_function_name
            -DTEST_FN_TYPE=test_case_function_type
            -DTEST_FN_ARR_NAME=test_cases_functions_array_name
            -P this_script.cmake
            -- sources...
    
    For every line in sources with pattern
        "TEST_DEF(<other>..."
    generates file SUITE_HEADER
    
    #include TEST_HEADER_INCLUDE

    TEST_DECL(<other1>
    TEST_DECL(<other2>
    ...

    static TEST_FN_TYPE TEST_FN_ARR_NAME[] = {
        TEST_NAME(<other1>,
        TEST_NAME(<other2>,
        ...
        0
    };

]]

function (check_arg arg_name new_name)
    if (DEFINED ${arg_name})
        set(${new_name} ${${arg_name}} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "${arg_name} should be defined.")
    endif()
endfunction()

check_arg(SUITE_HEADER suite_header)
check_arg(TEST_HEADER_INCLUDE test_include)
check_arg(TEST_DEF def)
check_arg(TEST_DECL decl)
check_arg(TEST_NAME func_name)
check_arg(TEST_FN_TYPE fn_type)
check_arg(TEST_FN_ARR_NAME arr_name)

set(sources "")

set(in_sources FALSE)

foreach(i  RANGE ${CMAKE_ARGC})
    if (i LESS ${CMAKE_ARGC})
        set(arg ${CMAKE_ARGV${i}})
        if (in_sources)
            list(APPEND sources ${arg})
        endif()
        if (arg STREQUAL "--")
            set(in_sources TRUE)
        endif()
    endif()
endforeach()
    
set(test_lines "")

string(APPEND def "(")

string(LENGTH ${def} pad_len)
string(REPEAT " " ${pad_len} pad)

foreach(file ${sources})
    file(READ ${file} src)
    string(REPLACE "\n" ";" src ${src})
    foreach(line ${src})
        set(line_copy ${line})

        # checking if line starts with ${TEST_DEF}
        # and appending rest of the line to test_lines
        string(APPEND line ${pad})
        string(SUBSTRING "${line}" 0 ${pad_len} line_begin)
        if (line_begin STREQUAL def)
            string(SUBSTRING "${line_copy}" ${pad_len} "-1" line_end)
            set(line_end "(${line_end}")
            list(APPEND test_lines ${line_end})
        endif()
    endforeach()
endforeach()

set(h "")

string(APPEND h "#include " ${test_include} "\n")

foreach(test ${test_lines})
    string(APPEND h "${decl}${test}\n")
endforeach()

string(APPEND h "static ${fn_type} ${arr_name}[] = {\n")

foreach(test ${test_lines})
    string(APPEND h "${func_name}${test},\n")
endforeach()

string(APPEND h "0};\n")

file(WRITE ${suite_header} "${h}")
