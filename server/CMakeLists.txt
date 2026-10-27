# Server logic lives in a library so tests can drive it without main().
# The server must never depend on SDL or rendering code.
add_library(wind_server_core STATIC
    src/core/server_options.cpp
    src/core/server_app.cpp)
target_include_directories(wind_server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_server_core PUBLIC wind::shared enet::enet)
wind_set_warnings(wind_server_core)

add_executable(wind_server src/main.cpp)
target_link_libraries(wind_server PRIVATE wind_server_core)
wind_set_warnings(wind_server)

if(WIND_BUILD_TESTS)
    add_executable(wind_server_tests
        tests/server_options_tests.cpp
        tests/server_app_tests.cpp)
    target_link_libraries(wind_server_tests PRIVATE wind_server_core GTest::gtest_main)
    wind_set_warnings(wind_server_tests)
    gtest_discover_tests(wind_server_tests)

    # Process-level smoke test: the real executable starts, ticks, and exits cleanly.
    add_test(NAME server_smoke_start_stop
             COMMAND wind_server --max-ticks 5 --tick-rate 50)
endif()
