configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wind/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/wind/version.hpp
    @ONLY)

add_library(wind_shared STATIC
    src/log.cpp
    src/cli.cpp)
add_library(wind::shared ALIAS wind_shared)

target_include_directories(wind_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(wind_shared PUBLIC spdlog::spdlog)
wind_set_warnings(wind_shared)

if(WIND_BUILD_TESTS)
    add_executable(wind_shared_tests
        tests/log_tests.cpp
        tests/cli_tests.cpp)
    target_link_libraries(wind_shared_tests PRIVATE wind::shared GTest::gtest_main)
    wind_set_warnings(wind_shared_tests)
    gtest_discover_tests(wind_shared_tests)
endif()
