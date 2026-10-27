# 프로젝트 최상위 빌드 파일. 공통 설정을 정하고 각 모듈을 불러온다.
cmake_minimum_required(VERSION 3.25)

project(ProjectWIND
    VERSION 0.0.1
    DESCRIPTION "Original 2D tile-based MMORPG with a custom engine"
    LANGUAGES C CXX)

# 모든 타깃은 표준 C++20을 사용한다 (컴파일러 확장 문법 금지).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# 에디터/정적 분석 도구용 compile_commands.json 생성
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 빌드할 범위를 끌 수 있는 스위치 (예: 서버 전용 빌드는 클라이언트 OFF)
option(WIND_BUILD_CLIENT "Build the SDL3 game client" ON)
option(WIND_BUILD_TESTS "Build unit tests" ON)

# 모든 실행 파일을 build/<preset>/bin 한 곳에 모아 스크립트와 ctest가 찾기 쉽게 한다.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(cmake/CompilerWarnings.cmake)  # wind_set_warnings() 정의
include(cmake/Dependencies.cmake)      # 외부 라이브러리 다운로드·고정

if(WIND_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)  # gtest_discover_tests() 사용
endif()

# 의존 방향: client/server -> shared (shared는 아무것도 의존하지 않는다)
add_subdirectory(shared)
add_subdirectory(server)
if(WIND_BUILD_CLIENT)
    add_subdirectory(client)
endif()
