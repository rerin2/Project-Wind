# 이 파일은 무엇을 하나요?
# 게임 클라이언트를 "어떤 재료(소스 파일)로, 어떤 묶음으로" 만들지 적은 조립 설명서예요.
# 레고처럼 작은 묶음(라이브러리)을 먼저 만들고, 그것들을 합쳐 실행 파일(.exe)을 만들어요.
#
# SDL3(창·키보드·그리기 도구 상자)는 engine/platform, engine/render(renderer.cpp),
# engine/resources 안에서만 써요. 나머지 엔진·게임 코드는 SDL 없이도 테스트할 수 있어요.
# (창을 띄우지 않고 숙제 검사를 할 수 있다는 뜻이에요.)

# 묶음 1: 실행 설정 읽기 (SDL 없음). 기본 자료 폴더는 저장소의 content/ 폴더예요.
add_library(wind_client_options STATIC src/app/client_options.cpp)
target_include_directories(wind_client_options PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_client_options PUBLIC wind::shared)
# 코드 안의 WIND_DEFAULT_CONTENT_DIR 자리에 content 폴더의 실제 위치를 채워 넣어요.
target_compile_definitions(wind_client_options PUBLIC
    WIND_DEFAULT_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content")
wind_set_warnings(wind_client_options)

# 묶음 2: 엔진 핵심 (SDL 없음): 박자 시계, 좌표 계산, 카메라, 키보드 출석부,
# 애니메이션, 그리기 순서표, 타일맵 배치
add_library(wind_engine_core STATIC
    src/engine/core/frame_clock.cpp
    src/engine/map/coordinates.cpp
    src/engine/map/tilemap_renderer.cpp
    src/engine/camera/camera2d.cpp
    src/engine/input/input_state.cpp
    src/engine/animation/animation.cpp
    src/engine/animation/sprite_sheet_loader.cpp
    src/engine/render/render_queue.cpp)
target_include_directories(wind_engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_engine_core PUBLIC wind::shared)
wind_set_warnings(wind_engine_core)

# 묶음 3: 게임 핵심 (SDL 없음): 한 칸 이동, 키 버튼표
add_library(wind_game_core STATIC
    src/game/entities/grid_mover.cpp
    src/game/input/input_bindings.cpp)
target_link_libraries(wind_game_core PUBLIC wind_engine_core)
wind_set_warnings(wind_game_core)

# 최종 결과물: 클라이언트 실행 파일. SDL3를 파일 안에 함께 넣어서(정적 링크) DLL 없이 실행돼요.
add_executable(wind_client
    src/main.cpp
    src/app/client_app.cpp
    src/app/debug_overlay.cpp
    src/engine/platform/sdl_platform.cpp
    src/engine/render/renderer.cpp
    src/engine/resources/resource_manager.cpp
    src/game/states/field_state.cpp)
target_link_libraries(wind_client PRIVATE wind_client_options wind_game_core SDL3::SDL3)
wind_set_warnings(wind_client)

if(WIND_BUILD_TESTS)
    # 숙제 검사(테스트) 프로그램. 창을 열지 않아요.
    # 창이 잘 뜨는지는 --max-frames로 실제 실행해서 따로 확인해요.
    add_executable(wind_client_tests
        tests/client_options_tests.cpp
        tests/frame_clock_tests.cpp
        tests/coordinates_tests.cpp
        tests/camera_tests.cpp
        tests/input_tests.cpp
        tests/animation_tests.cpp
        tests/render_queue_tests.cpp
        tests/tilemap_renderer_tests.cpp
        tests/grid_mover_tests.cpp)
    target_link_libraries(wind_client_tests PRIVATE wind_client_options wind_game_core GTest::gtest_main)
    # 테스트가 저장소의 진짜 자료(content 폴더)를 찾을 수 있게 위치를 알려 줘요.
    target_compile_definitions(wind_client_tests PRIVATE
        WIND_TEST_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content")
    wind_set_warnings(wind_client_tests)
    gtest_discover_tests(wind_client_tests)  # TEST 하나하나를 ctest 목록에 올려요
endif()
