// 이 파일은 무엇을 하나요?
// 필드 화면 준비물을 파일에서 차례로 읽고 검사하는 곳이에요.

#include "game/states/field_assets.hpp"

#include "engine/animation/sprite_sheet_loader.hpp"
#include "engine/map/coordinates.hpp"

#include "wind/content/day_cycle_loader.hpp"
#include "wind/content/spell_loader.hpp"
#include "wind/content/weapon_loader.hpp"
#include "wind/content/error_list.hpp"
#include "wind/content/monster_loader.hpp"
#include "wind/content/training_dummy_loader.hpp"
#include "wind/content/weather_loader.hpp"
#include "wind/log.hpp"

#include <map>
#include <string>
#include <utility>

namespace wind::game {
namespace {

// 주인공 그림 묶음(스프라이트 시트)에 꼭 있어야 하는 동작 그림(클립) 이름
constexpr std::string_view kIdleClip = "idle";  // 가만히 서 있기
constexpr std::string_view kWalkClip = "walk";  // 걷기
// 시작 무기 (명세 8.1). 장비 화면이 생기면(단계 15) 저장된 장비에서 골라요.
constexpr std::string_view kStartWeapon = "weapons/longsword.json";
// 시작 마술 (명세 8.6). 마술 칸이 생기면(단계 15) 저장된 칸에서 골라요.
constexpr std::string_view kStartSpell = "spells/star_shard.json";

// 맵의 허수아비 자리마다 설명 파일을 읽어 허수아비를 만들어요.
// 같은 파일은 한 번만 읽어요. 하나라도 잘못되면 이유를 기록하고 "없음"을 돌려줘요.
std::optional<std::vector<TrainingDummy>> load_dummies(const std::filesystem::path& content_root,
                                                       const world::MapData& map) {
    std::vector<TrainingDummy> dummies;                                // 만든 허수아비들
    std::map<std::string, combat::TrainingDummyData> cache;            // 파일 위치 -> 읽은 설명
    for (const auto& entity : map.entities) {                          // 맵 물건을 하나씩 보며
        if (entity.type != world::EntityType::TrainingDummy) continue;  // 허수아비만
        auto it = cache.find(entity.data);                             // 이미 읽은 파일인가
        if (it == cache.end()) {
            auto loaded = content::load_training_dummy_file(content_root, entity.data);  // 처음이면 읽기
            if (!loaded.ok()) {
                content::log_content_errors("training dummy " + entity.data, loaded.errors);
                return std::nullopt;
            }
            it = cache.emplace(entity.data, std::move(*loaded.value)).first;
        }
        const auto target = static_cast<TargetId>(dummies.size() + 1);  // 대상 번호 (1부터)
        dummies.emplace_back(target, entity.id, it->second,
                             engine::tile_feet(entity.tile, map.tile_size));  // 칸 위에 세우기
    }
    return dummies;
}

// 맵의 몬스터 자리마다 설명 파일을 읽어 몬스터를 만들어요. (11a)
// 몬스터는 칸을 막지 않아서, 설명 파일이 잘못되면 그 몬스터만 빼고(기록은 남겨요) 맵은 그대로 열어요.
std::vector<Monster> load_monsters(const std::filesystem::path& content_root, const world::MapData& map) {
    std::vector<Monster> monsters;                                    // 만든 몬스터들
    std::map<std::string, std::optional<combat::MonsterData>> cache;  // 파일 위치 -> 읽은 설명 (실패도 기억)
    for (const auto& entity : map.entities) {                         // 맵 물건을 하나씩 보며
        if (entity.type != world::EntityType::Monster) continue;      // 몬스터만
        auto it = cache.find(entity.data);                            // 이미 읽은 파일인가
        if (it == cache.end()) {
            auto loaded = content::load_monster_file(content_root, entity.data);  // 처음이면 읽기
            if (!loaded.ok()) {
                content::log_content_errors("monster " + entity.data, loaded.errors);  // 한 번만 기록
            }
            it = cache.emplace(entity.data, std::move(loaded.value)).first;
        }
        if (!it->second) {
            log::warn("Monster '{}' skipped: bad data file '{}'", entity.id, entity.data);
            continue;  // 잘못된 설명이면 이 몬스터만 빼요
        }
        monsters.emplace_back(entity.id, *it->second, engine::tile_feet(entity.tile, map.tile_size));
    }
    return monsters;
}

// 맵의 쿠션 자리마다 쿠션 정보를 만들어요 (13b). 이름이 없으면 지역 이름을 써요.
std::vector<Cushion> load_cushions(const world::MapData& map) {
    std::vector<Cushion> cushions;
    for (const auto& entity : map.entities) {
        if (entity.type != world::EntityType::SavePoint) continue;  // 쿠션만
        const std::string& name = entity.display_name.empty() ? map.display_name : entity.display_name;
        cushions.push_back({entity.id, name, entity.tile, engine::tile_feet(entity.tile, map.tile_size)});
    }
    return cushions;
}

// 등장 자리 이름으로 시작 칸을 찾아요. 순서: 등장 자리 -> 쿠션 id(13b: 쉬기·부활) -> "default"면 첫 자리.
// 쿠션에서 시작하면 그 쿠션 id도 알려 줘요 (주인공이 앉은 채 시작해요).
struct StartPlace {
    world::TileCoord tile;
    std::optional<std::string> cushion;
};
std::optional<StartPlace> find_start(const world::MapData& map, const std::vector<Cushion>& cushions,
                                     std::string_view spawn_id) {
    if (const world::SpawnPoint* spawn = map.find_spawn(spawn_id)) return StartPlace{spawn->position, {}};
    for (const auto& cushion : cushions) {
        if (cushion.id == spawn_id) return StartPlace{cushion.tile, cushion.id};
    }
    if (spawn_id == "default" && !map.spawn_points.empty()) return StartPlace{map.spawn_points.front().position, {}};
    return std::nullopt;  // 맵 자료(또는 저장된 부활 자리)가 틀렸어요
}

}  // namespace

std::optional<FieldAssets> load_field_assets(const std::filesystem::path& content_root,
                                             std::string_view map_path, std::string_view sheet_path,
                                             std::string_view spawn_id,
                                             std::uint32_t stress_particles) {
    // 1) 맵 파일(과 맵이 쓰는 타일셋)을 읽고 검사해요.
    auto map = content::load_map_file(content_root, map_path);
    if (!map.ok()) {
        content::log_content_errors("map", map.errors);
        return std::nullopt;
    }
    // 2) 주인공 그림 설명서(스프라이트 시트 JSON)를 읽고 검사해요.
    auto sheet = engine::load_sprite_sheet_file(content_root, sheet_path);
    if (!sheet.ok()) {
        content::log_content_errors("player sprite sheet", sheet.errors);
        return std::nullopt;
    }
    // 3) "서 있기"와 "걷기" 그림이 둘 다 있어야 해요.
    if (sheet.value->clip(kIdleClip) == nullptr || sheet.value->clip(kWalkClip) == nullptr) {
        log::error("Player sprite sheet '{}' needs '{}' and '{}' clips", sheet_path, kIdleClip,
                   kWalkClip);
        return std::nullopt;
    }
    // 4) 등장 자리: 요청한 이름의 등장 자리나 쿠션(13b). "default"를 요청했는데 없으면 첫 번째 자리.
    //    다른 이름(출구가 가리킨 자리)이 없으면 맵 자료가 틀린 거라 들어가지 않아요.
    auto cushions = load_cushions(map.value->map);
    const auto start = find_start(map.value->map, cushions, spawn_id);
    if (!start) {
        log::error("Map '{}' has no spawn point or cushion '{}'", map_path, spawn_id);
        return std::nullopt;
    }
    const world::TileCoord spawn_tile = start->tile;

    // 5) 바깥 맵이면 하루 색 곡선과 날씨 표 (잘못되면 늘 낮·늘 맑음)
    const auto& ambience = map.value->map.ambience;
    std::optional<world::DayCycle> day_cycle;
    if (!ambience.day_cycle.empty()) {
        if (auto loaded = content::load_day_cycle_file(content_root, ambience.day_cycle); loaded.ok()) {
            day_cycle = std::move(*loaded.value);
        } else {
            content::log_content_errors("day cycle " + ambience.day_cycle, loaded.errors);
        }
    }
    std::optional<world::WeatherTable> weather;
    if (!ambience.weather.empty()) {
        if (auto loaded = content::load_weather_file(content_root, ambience.weather); loaded.ok()) {
            weather = std::move(*loaded.value);
        } else {
            content::log_content_errors("weather " + ambience.weather, loaded.errors);
        }
    }

    // 6) 파티클 효과들 (잘못된 파일은 건너뛰어요)
    FieldEffects effects = load_field_effects(content_root, ambience, weather ? &*weather : nullptr,
                                              stress_particles);

    // 7) 든 무기 (잘못되면 기록하고 맨손: 공격만 안 나가고 나머지는 그대로 돼요)
    combat::Weapon weapon;
    if (auto loaded = content::load_weapon_file(content_root, kStartWeapon); loaded.ok()) {
        weapon = std::move(*loaded.value);
    } else {
        content::log_content_errors("weapon " + std::string(kStartWeapon), loaded.errors);
    }

    // 8) 맵에 놓인 허수아비들 (설명 파일이 잘못되면 맵을 열지 않아요: 막힌 칸만 남는 걸 막으려고)
    auto dummies = load_dummies(content_root, map.value->map);
    if (!dummies) {
        return std::nullopt;  // 이유는 이미 기록했어요
    }

    // 9) 고른 마술 (잘못되면 기록하고 마술 없음: L/C만 안 되고 나머지는 그대로 돼요)
    std::optional<combat::Spell> spell;
    if (auto loaded = content::load_spell_file(content_root, kStartSpell); loaded.ok()) {
        spell = std::move(*loaded.value);
    } else {
        content::log_content_errors("spell " + std::string(kStartSpell), loaded.errors);
    }

    // 10) 맵의 몬스터들 (11a). 아래에서 맵을 넘겨주기(move) 전에 읽어야 해요.
    auto monsters = load_monsters(content_root, map.value->map);
    std::vector<std::string> monster_fx;  // 몬스터가 사라질 때 효과 파일들 (11d)
    for (const auto& monster : monsters) {
        if (monster.data().death_fx) monster_fx.push_back(*monster.data().death_fx);
    }
    load_monster_effects(content_root, monster_fx, effects);

    return FieldAssets{std::move(*map.value), std::move(*sheet.value), spawn_tile, std::move(effects),
                       std::move(day_cycle), std::move(weather), std::move(weapon),
                       std::move(*dummies), std::move(spell),
                       std::move(monsters), std::move(cushions), start->cushion};
}

void log_field_entry(const FieldAssets& assets) {
    const auto& map = assets.map.map;
    log::info("Entered map '{}' ({}x{} tiles, {} ambient effect(s), {} light(s), lighting {}, "
              "day cycle {}, weather {})",
              map.map_id, map.width, map.height, assets.effects.ambient.size(),
              map.ambience.lights.size(), map.ambience.ambient_light ? "fixed" : "off",
              assets.day_cycle ? "on" : "off",
              assets.weather ? std::to_string(assets.weather->states.size()) + " state(s)"
                             : std::string("off"));
    if (!assets.dummies.empty()) {
        log::info("Training dummies: {}", assets.dummies.size());
    }
    if (!assets.monsters.empty()) {
        log::info("Monsters: {}", assets.monsters.size());
    }
}

}  // namespace wind::game
