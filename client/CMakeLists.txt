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

# 묶음 2: 엔진 핵심 (SDL 없음): 박자 시계, 좌표 계산, 카메라(+앞보기), 키보드 출석부,
# 애니메이션, 그리기 순서표, 타일맵 배치, 파티클(먼지 같은 작은 점)
add_library(wind_engine_core STATIC
    src/engine/core/frame_clock.cpp
    src/engine/map/coordinates.cpp
    src/engine/map/tilemap_renderer.cpp
    src/engine/camera/camera2d.cpp
    src/engine/camera/look_ahead.cpp
    src/engine/input/input_state.cpp
    src/engine/animation/sprite_sheet.cpp
    src/engine/animation/animator.cpp
    src/engine/animation/sprite_sheet_loader.cpp
    src/engine/render/render_queue.cpp
    src/engine/fx/particle_pool.cpp
    src/engine/fx/particle_drawing.cpp)
target_include_directories(wind_engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wind_engine_core PUBLIC wind::shared)
wind_set_warnings(wind_engine_core)

# 묶음 3: 게임 핵심 (SDL 없음)
#   input/        : 키 버튼표, 가고 싶은 방향, 버튼 메모장(입력 버퍼)
#   physics/      : 발 상자와 벽 충돌 (미끄러지기, 모서리 보정)
#   entities/     : 캐릭터 다리(Mover), 바라보는 방향
#   presentation/ : 먼지, 잔상 같은 보여 주기용 효과 (그리기 목록에 넣기까지만, SDL은 안 씀)
add_library(wind_game_core STATIC
    src/game/input/input_bindings.cpp
    src/game/input/movement_intent.cpp
    src/game/input/input_buffer.cpp
    src/game/physics/tile_collision.cpp
    src/game/entities/facing.cpp
    src/game/entities/mover.cpp
    src/game/presentation/dust_emitter.cpp
    src/game/presentation/afterimage_trail.cpp)
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
        tests/input_state_tests.cpp
        tests/input_bindings_tests.cpp
        tests/movement_intent_tests.cpp
        tests/input_buffer_tests.cpp
        tests/animator_tests.cpp
        tests/sprite_sheet_loader_tests.cpp
        tests/render_queue_tests.cpp
        tests/tilemap_renderer_tests.cpp
        tests/tile_collision_tests.cpp
        tests/facing_tests.cpp
        tests/mover_tests.cpp
        tests/particle_pool_tests.cpp
        tests/look_ahead_tests.cpp
        tests/dust_emitter_tests.cpp)
    target_link_libraries(wind_client_tests PRIVATE wind_client_options wind_game_core GTest::gtest_main)
    # 테스트가 저장소의 진짜 자료(content 폴더)를 찾을 수 있게 위치를 알려 줘요.
    target_compile_definitions(wind_client_tests PRIVATE
        WIND_TEST_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content")
    wind_set_warnings(wind_client_tests)
    gtest_discover_tests(wind_client_tests)  # TEST 하나하나를 ctest 목록에 올려요
endif()
