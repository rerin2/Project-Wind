cmake_minimum_required(VERSION 3.25)

project(ProjectWIND
    VERSION 0.0.1
    DESCRIPTION "Original 2D tile-based MMORPG with a custom engine"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WIND_BUILD_CLIENT "Build the SDL3 game client" ON)
option(WIND_BUILD_TESTS "Build unit tests" ON)

# All executables land in one folder so scripts and ctest can find them.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/CompilerWarnings.cmake)
include(cmake/Dependencies.cmake)

if(WIND_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)
endif()

add_subdirectory(shared)
add_subdirectory(server)
if(WIND_BUILD_CLIENT)
    add_subdirectory(client)
endif()
