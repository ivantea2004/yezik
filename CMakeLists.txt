
cmake_minimum_required(VERSION 3.10)

project(yezik C)

option(BUILD_TESTING "Build tests" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -pedantic -pedantic-errors -Wconversion -Wsign-conversion")

set(modules
    assert
    cli
)

set(headers "")
set(sources "")
set(tests "")

foreach(m ${modules})
    list(APPEND headers ${CMAKE_CURRENT_SOURCE_DIR}/include/yezik/${m}.h)
    list(APPEND sources ${CMAKE_CURRENT_SOURCE_DIR}/src/${m}.c)
    list(APPEND tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/${m}_test.c)
endforeach()

set(includes ${CMAKE_CURRENT_SOURCE_DIR}/include)

message("1 = " ${headers})
message("2 = " ${sources})
message("3 = " ${tests})

add_executable(yezik
    src/main.c
    ${sources}
    ${headers}
)

target_include_directories(yezik PRIVATE ${includes})

if(BUILD_TESTING)
    enable_testing()
    include(tests/helpers/cmake/test_setup.cmake)
    setup_tests_from_files(
        TEST_TARGET yezik_test
        DEFINITIONS ""
        INCLUDE_DIRS ${includes} ${CMAKE_CURRENT_SOURCE_DIR}
        LIBRARIES ""
        SOURCES ${tests}
        CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_config.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
