# server 모듈: 권위 게임 서버. SDL이나 렌더링 코드에 절대 의존하지 않는다.

# 서버 로직은 라이브러리로 분리해 main() 없이도 테스트에서 직접 실행할 수 있게 한다.
add_library(wind_server_core STATIC
    src/core/server_options.cpp
    src/core/server_app.cpp)
target_include_directories(wind_server_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_server_core PUBLIC wind::shared enet::enet)
wind_set_warnings(wind_server_core)

# 실제 서버 실행 파일: 인자 처리, 신호 처리, ServerApp 실행만 담당한다.
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

    # 프로세스 단위 스모크 테스트: 실제 실행 파일이 시작해 5틱 돈 뒤 정상 종료(0)하는지 확인
    add_test(NAME server_smoke_start_stop
             COMMAND wind_server --max-ticks 5 --tick-rate 50)
endif()
