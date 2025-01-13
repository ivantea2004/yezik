#[[

    This file is a cmake script to discover test and generate a test suite.

    Intended to be called like:
        cmake
            -DSUITE=path/to/suite.c
            -DCONFIG_FILE=path/to/config.h
            -P this_script.cmake
            -- sources...
    
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

    For every line in sources with pattern
        "TEST_DEF(<other>"
    generates file SUITE file
    
    #include <CONFIG_FILE>

    TEST_DECL(<other1>
    TEST_DECL(<other2>
    ...

    static TEST_FN_TYPE test_suite[] = {
        TEST_FN_NAME(<other1>,
        TEST_FN_NAME(<other2>,
        ...
        0
    };

]]

function(check_arg arg_name new_name)
    if(DEFINED ${arg_name})
        set(${new_name} ${${arg_name}} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "${arg_name} should be defined.")
    endif()
endfunction()

check_arg(SUITE suite)
check_arg(CONFIG_FILE config_path)

message("config = " ${config})
message("suite = " ${suite})

#[[
#define<space>X<space>...
#define<space>X(...
]]

# returns <ret> and <ret>_REM
function(starts_with str other ret)
    string(FIND ${str} ${other} place)

    if(NOT(place STREQUAL "0"))
        set(${ret} FALSE PARENT_SCOPE)
    else()
        string(LENGTH ${other} len)
        string(SUBSTRING ${str} ${len} "-1" end)
        set(${ret}_REM ${end} PARENT_SCOPE)
        set(${ret} TRUE PARENT_SCOPE)
    endif()
endfunction()

# returns prefix of <str> before first encounter of <other>
function(cut_before str other ret)
    # string(FIND <string> <substring> <output_variable> [REVERSE])
    string(FIND ${str} ${other} out)

    if(${out} STREQUAL "-1")
        set(${ret} ${str} PARENT_SCOPE)
    else()
        # string(SUBSTRING <string> <begin> <length> <output_variable>)
        string(SUBSTRING ${str} 0 ${out} b)
        set(${ret} ${b} PARENT_SCOPE)
    endif()
endfunction()

#[[
    checks if line is
    #define X ...
    #define X(...
    returns <out> as TRUE or FALSE
    returns X in <out>_MACRO 
]]
function(is_macro_definition_line line out)
    message("out = ${out}")
    starts_with(${line} "#define " s)
    message("out = ${out}")
    message("line = ${line}")

    if(s)
        message("at ${s_REM}")
        cut_before(${s_REM} "(" res)
        message("at ${res}")
        cut_before(${res} " " res)
        message("at ${res}")
        message("out = ${out}")
        set(${out} "TRUE" PARENT_SCOPE)
        message("out = ${out}")
        set(${out}_MACRO ${res} PARENT_SCOPE)

    # message("set out = ${out}")
    else()
        set(${out} FALSE PARENT_SCOPE)
    endif()
endfunction()

# returns call args in <out>_ARGS
function(is_macro_call_line line m out)
    starts_with(${line} "${m}(" res)

    if(res)
        set(${out}_ARGS "(${res_REM}" PARENT_SCOPE)
        set(${out} TRUE PARENT_SCOPE)
    else()
        set(${out} FALSE PARENT_SCOPE)
    endif()
endfunction()

cut_before("123" "4" out)
cut_before("some 14_123" "4" out)

starts_with("abc" "ab" out)

is_macro_definition_line("#define X(lolkek" out)

message("out = ${out}, rem = ${out_MACRO}")

file(READ ${config_path} config)
string(REPLACE "\n" ";" config "${config}")
string(REPLACE "\\" ";" config "${config}")
message("config = ${config}")
set(config_macros "")

foreach(line ${config})
    is_macro_definition_line("${line}" out)
    message("out = ${out}, out_MACRO=${out_MACRO}")

    if(out)
        list(APPEND config_macros ${out_MACRO})
    endif()
endforeach()

message("macros = ${config_macros}")

list(LENGTH config_macros config_macros_length)

if(config_macros_length LESS "4")
    message(FATAL_ERROR "Not enough macro definitions or they can not be properly parsed in file ${config_path}")
endif()

list(GET config_macros 0 def)
list(GET config_macros 1 decl)
list(GET config_macros 2 func_name)
list(GET config_macros 3 func_type)

message("${def} ${decl} ${func_name} ${func_type}")

set(sources "")

set(in_sources FALSE)

foreach(i RANGE ${CMAKE_ARGC})
    if(i LESS ${CMAKE_ARGC})
        set(arg ${CMAKE_ARGV${i}})

        if(in_sources)
            list(APPEND sources ${arg})
        endif()

        if(arg STREQUAL "--")
            set(in_sources TRUE)
        endif()
    endif()
endforeach()

set(test_args "")

foreach(file ${sources})
    file(READ ${file} src)

    if(NOT(src STREQUAL ""))
        string(REPLACE "\n" ";" src "${src}")

        foreach(line ${src})
            is_macro_call_line("${line}" ${def} out)

            if(out)
                list(APPEND test_args ${out_ARGS})
            endif()
        endforeach()
    endif()
endforeach()

message("test_args = ${test_args}")

get_filename_component(config_file ${config_path} NAME)

set(h "")

string(APPEND h "#include \"${config_file}\"\n")

foreach(test ${test_args})
    string(APPEND h "${decl}${test}\n")
endforeach()

string(APPEND h "static ${func_type} test_suite[] = {\n")

foreach(test ${test_args})
    string(APPEND h "${func_name}${test},\n")
endforeach()

string(APPEND h "0};\n")

file(WRITE ${suite} "${h}")
