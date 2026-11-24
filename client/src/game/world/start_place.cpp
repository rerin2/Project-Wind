// 이 파일은 무엇을 하나요?
// 시작 칸 찾기와 쿠션 목록 만들기 (start_place.hpp 설명 참고).

#include "game/world/start_place.hpp"

#include "engine/map/coordinates.hpp"

namespace wind::game {

std::vector<Cushion> load_cushions(const world::MapData& map) {
    std::vector<Cushion> cushions;
    for (const auto& entity : map.entities) {
        if (entity.type != world::EntityType::SavePoint) continue;  // 쿠션만
        const std::string& name = entity.display_name.empty() ? map.display_name : entity.display_name;
        cushions.push_back({entity.id, name, entity.tile, engine::tile_feet(entity.tile, map.tile_size)});
    }
    return cushions;
}

std::optional<StartPlace> find_start(const world::MapData& map, const std::vector<Cushion>& cushions,
                                     std::string_view spawn_id) {
    if (const world::SpawnPoint* spawn = map.find_spawn(spawn_id)) return StartPlace{spawn->position, {}};
    for (const auto& cushion : cushions) {
        if (cushion.id == spawn_id) return StartPlace{cushion.tile, cushion.id};
    }
    if (spawn_id == "default" && !map.spawn_points.empty()) return StartPlace{map.spawn_points.front().position, {}};
    return std::nullopt;
}

}  // namespace wind::game
