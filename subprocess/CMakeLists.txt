#[[
    Simple header only library to run subprocesses
]]
cmake_minimum_required(VERSION 3.10)

project(subrocess)

add_library(subprocess_subprocess INTERFACE)

target_include_directories(subprocess_subprocess INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(subprocess::subprocess ALIAS subprocess_subprocess)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
