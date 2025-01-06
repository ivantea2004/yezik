
cmake_minimum_required(VERSION 3.10)

project(yezik C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -pedantic -pedantic-errors -Wconversion -Wsign-conversion")

add_subdirectory(src)

