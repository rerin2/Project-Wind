# Third-party dependencies, fetched at configure time and pinned to exact tags.
# Record any change here in DEPENDENCIES.md (version, license, rationale).
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

# --- spdlog: logging backend behind wind::log ------------------------------
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3
    GIT_SHALLOW    TRUE)

# --- ENet: UDP transport (application protocol is ours) --------------------
# ENet's CMakeLists predates CMake 3.5; CMake 4 refuses it without this floor.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
FetchContent_Declare(enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        v1.3.18
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(spdlog enet)
unset(CMAKE_POLICY_VERSION_MINIMUM)

# ENet does not publish usage requirements; add them so consumers just link.
target_include_directories(enet SYSTEM INTERFACE ${enet_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(enet INTERFACE ws2_32 winmm)
endif()
add_library(enet::enet ALIAS enet)

# --- SDL3: window, input, renderer, audio (client only) --------------------
if(WIND_BUILD_CLIENT)
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

# --- GoogleTest: unit tests ------------------------------------------------
if(WIND_BUILD_TESTS)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(googletest)
endif()
