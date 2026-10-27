# 외부 라이브러리. configure 시점에 내려받고 정확한 태그로 버전을 고정한다.
# 여기를 바꾸면 DEPENDENCIES.md(버전·라이선스·도입 이유)도 함께 갱신한다.
include(FetchContent)

# 다운로드 진행 상황을 출력해 첫 configure가 멈춘 것처럼 보이지 않게 한다.
set(FETCHCONTENT_QUIET OFF)

# --- spdlog: wind::log 뒤에서 동작하는 로그 백엔드 -----------------------------
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)  # 정적 라이브러리로 빌드
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3
    GIT_SHALLOW    TRUE)

# --- ENet: UDP 전송 계층 (게임 프로토콜은 우리가 직접 정의) ------------------------
# ENet의 CMakeLists는 CMake 3.5 이전 문법이라, CMake 4는 이 하한값 없이 거부한다.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.18
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(spdlog enet)
unset(CMAKE_POLICY_VERSION_MINIMUM)  # 다른 라이브러리에는 적용하지 않는다

# ENet은 include 경로·링크 라이브러리를 타깃에 노출하지 않으므로 직접 추가한다.
# 이렇게 하면 사용하는 쪽은 enet::enet만 링크하면 된다.
target_include_directories(enet SYSTEM INTERFACE ${enet_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(enet INTERFACE ws2_32 winmm)  # 윈속, 타이머
endif()
add_library(enet::enet ALIAS enet)

# --- SDL3: 창·입력·렌더러·오디오 (클라이언트 전용) ---------------------------------
if(WIND_BUILD_CLIENT)
    # 정적 링크로 SDL3.dll 배포가 필요 없게 하고, 테스트·예제 빌드는 끈다.
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.4.18
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(SDL3)
endif()

# --- GoogleTest: 단위 테스트 -------------------------------------------------
if(WIND_BUILD_TESTS)
    # 우리 코드와 같은 동적 CRT(/MD)를 쓰도록 맞춘다. 다르면 링크 오류가 난다.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)  # 아직 mock은 쓰지 않는다
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(googletest)
endif()
