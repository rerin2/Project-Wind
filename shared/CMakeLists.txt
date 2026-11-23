# shared 모듈: 클라이언트·도구가 함께 쓰는 SDL 없는 코드.
# client에 의존하면 안 된다.

# version.hpp.in의 @PROJECT_VERSION@을 채워 빌드 폴더에 헤더를 생성한다.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/wind/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/wind/version.hpp
    @ONLY)

# 공용 코드 묶음(라이브러리: 여러 프로그램이 빌려 쓰는 코드 꾸러미)
#   world/   : 바둑판 맵과 방향 정보
#   content/ : 설명서(JSON) 읽기, 숙제 검사, 파일 위치 문지기
#   combat/  : 공격·무기·마술·몬스터 데이터, 피해 계산 (SDL 없는 순수 규칙)
#   progression/ : 능력치·곡선·파생값 계산·레벨 비용·룬 지갑 (13a, 순수 규칙)
#   items/   : 아이템 도감·인벤토리 규칙 (14a, 순수 규칙)
add_library(wind_shared STATIC
    src/log.cpp
    src/cli.cpp
    src/world/direction.cpp
    src/world/map.cpp
    src/world/day_cycle.cpp
    src/content/content_path.cpp
    src/content/error_list.cpp
    src/content/json_file.cpp
    src/content/hex_color.cpp
    src/content/ambience_loader.cpp
    src/content/exit_loader.cpp
    src/content/day_cycle_loader.cpp
    src/content/weather_loader.cpp
    src/content/json_reader.cpp
    src/content/tileset_loader.cpp
    src/content/map_loader.cpp
    src/content/map_checks.cpp
    src/content/entity_loader.cpp
    src/content/attack_loader.cpp
    src/content/combat_readers.cpp
    src/content/weapon_loader.cpp
    src/content/spell_loader.cpp
    src/content/training_dummy_loader.cpp
    src/content/body_readers.cpp
    src/content/monster_loader.cpp
    src/content/progression_loader.cpp
    src/content/item_catalog_loader.cpp
    src/content/drop_table_reader.cpp
    src/combat/damage.cpp
    src/combat/heavy_weapon.cpp
    src/progression/stats.cpp
    src/progression/stat_curve.cpp
    src/progression/derived_stats.cpp
    src/progression/level_cost.cpp
    src/progression/rune_wallet.cpp
    src/items/item_catalog.cpp
    src/items/inventory.cpp
    src/items/equipment.cpp)
add_library(wind::shared ALIAS wind_shared)

# 사용하는 쪽은 #include "wind/..." 형태로 헤더를 쓴다 (생성된 헤더 포함).
target_include_directories(wind_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/generated)
# 공개 헤더가 spdlog(로그 도구)·nlohmann/json(설명서 읽는 도구) 헤더를 포함하므로,
# 이 묶음을 쓰는 쪽에도 함께 빌려준다(PUBLIC).
target_link_libraries(wind_shared PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json)
wind_set_warnings(wind_shared)

if(WIND_BUILD_TESTS)
    # 공용 코드 시험지 모음
    add_executable(wind_shared_tests
        tests/log_tests.cpp
        tests/cli_tests.cpp
        tests/direction_tests.cpp
        tests/content_path_tests.cpp
        tests/map_loader_tests.cpp
        tests/tileset_loader_tests.cpp
        tests/json_reader_tests.cpp
        tests/hex_color_tests.cpp
        tests/ambience_loader_tests.cpp
        tests/exit_loader_tests.cpp
        tests/day_cycle_tests.cpp
        tests/day_cycle_loader_tests.cpp
        tests/weather_loader_tests.cpp
        tests/damage_tests.cpp
        tests/attack_loader_tests.cpp
        tests/weapon_loader_tests.cpp
        tests/spell_loader_tests.cpp
        tests/entity_loader_tests.cpp
        tests/map_checks_tests.cpp
        tests/training_dummy_loader_tests.cpp
        tests/monster_loader_tests.cpp
        tests/stats_tests.cpp
        tests/stat_curve_tests.cpp
        tests/derived_stats_tests.cpp
        tests/level_cost_tests.cpp
        tests/rune_wallet_tests.cpp
        tests/progression_loader_tests.cpp
        tests/item_catalog_tests.cpp
        tests/item_catalog_loader_tests.cpp
        tests/inventory_tests.cpp
        tests/monster_drops_tests.cpp
        tests/equipment_tests.cpp
        tests/heavy_weapon_tests.cpp)
    target_link_libraries(wind_shared_tests PRIVATE wind::shared GTest::gtest_main)
    # 진짜 시험 맵을 검사하는 시험이 저장소의 content 폴더 위치를 알 수 있게 알려 준다.
    target_compile_definitions(wind_shared_tests PRIVATE
        WIND_TEST_CONTENT_DIR="${PROJECT_SOURCE_DIR}/content")
    wind_set_warnings(wind_shared_tests)
    gtest_discover_tests(wind_shared_tests)  # 각 TEST를 ctest 항목으로 등록
endif()
