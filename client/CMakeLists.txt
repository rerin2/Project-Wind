# Option parsing has no SDL dependency so it can be unit-tested headlessly.
add_library(wind_client_options STATIC src/app/client_options.cpp)
target_include_directories(wind_client_options PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_client_options PUBLIC wind::shared)
wind_set_warnings(wind_client_options)

add_executable(wind_client
    src/main.cpp
    src/app/client_app.cpp
    src/engine/platform/sdl_platform.cpp)
target_include_directories(wind_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_client PRIVATE wind_client_options SDL3::SDL3)
wind_set_warnings(wind_client)

if(WIND_BUILD_TESTS)
    add_executable(wind_client_tests tests/client_options_tests.cpp)
    target_link_libraries(wind_client_tests PRIVATE wind_client_options GTest::gtest_main)
    wind_set_warnings(wind_client_tests)
    gtest_discover_tests(wind_client_tests)
endif()
