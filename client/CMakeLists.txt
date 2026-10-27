# client 모듈: 게임 클라이언트. SDL3는 engine/ 폴더 안에서만 사용한다.

# 옵션 파싱은 SDL에 의존하지 않으므로 별도 라이브러리로 두어 창 없이 테스트한다.
add_library(wind_client_options STATIC src/app/client_options.cpp)
target_include_directories(wind_client_options PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_client_options PUBLIC wind::shared)
wind_set_warnings(wind_client_options)

# 클라이언트 실행 파일 (SDL3 정적 링크)
add_executable(wind_client
    src/main.cpp
    src/app/client_app.cpp
    src/engine/platform/sdl_platform.cpp)
target_include_directories(wind_client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_client PRIVATE wind_client_options SDL3::SDL3)
wind_set_warnings(wind_client)

if(WIND_BUILD_TESTS)
    # 테스트는 창을 열지 않는다. 창 동작은 --max-frames로 수동 확인한다.
    add_executable(wind_client_tests tests/client_options_tests.cpp)
    target_link_libraries(wind_client_tests PRIVATE wind_client_options GTest::gtest_main)
    wind_set_warnings(wind_client_tests)
    gtest_discover_tests(wind_client_tests)
endif()
