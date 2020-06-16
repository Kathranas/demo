cmake_minimum_required(VERSION 3.1) 
project(demo)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# All code is relative to the code directory
include_directories(code)

if(WIN32)
	# Debugging information
	add_compile_options(/Zi)

	# Disable exceptions
	add_compile_definitions(_HAS_EXCEPTIONS=0)

	add_executable(demo WIN32 code/win32_unity.cpp)
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

