# shared 모듈: 클라이언트·서버·도구가 함께 쓰는 플랫폼 독립 코드.
# client나 server에 의존하면 안 된다.

# version.hpp.in의 @PROJECT_VERSION@을 채워 빌드 폴더에 헤더를 생성한다.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wind/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/wind/version.hpp
    @ONLY)

add_library(wind_shared STATIC
    src/log.cpp
    src/cli.cpp)
add_library(wind::shared ALIAS wind_shared)

# 사용하는 쪽은 #include "wind/..." 형태로 헤더를 쓴다 (생성된 헤더 포함).
target_include_directories(wind_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated)
# log.hpp가 spdlog 헤더를 포함하므로 PUBLIC으로 전달한다.
target_link_libraries(wind_shared PUBLIC spdlog::spdlog)
wind_set_warnings(wind_shared)

if(WIND_BUILD_TESTS)
    add_executable(wind_shared_tests
        tests/log_tests.cpp
        tests/cli_tests.cpp)
    target_link_libraries(wind_shared_tests PRIVATE wind::shared GTest::gtest_main)
    wind_set_warnings(wind_shared_tests)
    gtest_discover_tests(wind_shared_tests)  # 각 TEST를 ctest 항목으로 등록
endif()
