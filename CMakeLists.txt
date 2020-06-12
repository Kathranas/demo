cmake_minimum_required(VERSION 3.1)

project(demo)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Null down compile options
set("CMAKE_CXX_FLAGS" "")
set("CMAKE_CXX_FLAGS_DEBUG" "")

# All code is relative to the code directory
include_directories(code)

# Disable exceptions
add_compile_options(-fno-exceptions)

add_executable(demo code/linux_unity.cpp)
