// 이 파일은 무엇을 하나요?
// 맵 마무리 검사: 지나갈 수 있는 칸 계산, 등장 자리·물건·출구 위치 확인.

#include "wind/content/map_checks.hpp"

#include <string>
#include <vector>

namespace wind::content {
namespace {

// 칸마다 "지나갈 수 있나?"를 계산해요.
// 규칙: 카드가 하나라도 놓여 있고, 놓인 카드가 전부 "지나갈 수 있음"이면 OK.
// 카드가 하나도 없는 칸은 허공이라 못 가요. 카드 하나라도 벽이면 못 가요.
void compute_walkability(world::MapData& map, const world::Tileset& tileset) {
    const auto cells = static_cast<std::size_t>(map.width) * static_cast<std::size_t>(map.height);
    map.walkable.assign(cells, 0);  // 처음엔 모두 "못 가요"로 채워 두고 하나씩 확인
    for (std::size_t i = 0; i < cells; ++i) {
        bool any_tile = false;  // 카드가 하나라도 있나
        bool blocked = false;   // 막는 카드가 있나
        for (const auto& layer : map.layers) {         // 같은 칸에 겹친 필름들을 모두 봐요
            const world::TileId tile = layer.tiles[i];  // 이 필름의 이 칸 카드
            if (tile == world::kEmptyTile) continue;    // 빈 칸은 건너뛰기
            any_tile = true;
            const auto* props = tileset.find(tile);                         // 카드 성질
            blocked = blocked || props == nullptr || !props->walkable;      // 모르거나 벽이면 막힘
        }
        map.walkable[i] = (any_tile && !blocked) ? 1 : 0;  // 결과 적기
    }
}

// 출구 영역 안에 걸어 들어갈 수 있는 칸이 하나라도 있나요?
bool exit_is_reachable(const world::MapData& map, const world::MapExit& exit) {
    for (std::int32_t y = exit.area_min.y; y <= exit.area_max.y; ++y) {
        for (std::int32_t x = exit.area_min.x; x <= exit.area_max.x; ++x) {
            if (map.is_walkable({x, y})) return true;  // 하나라도 있으면 OK
        }
    }
    return false;  // 전부 막혀 있어요
}

// "목록이름[번호]" 주소 만들기 (오류 메시지용)
std::string item_path(const char* list, std::size_t index) {
    return std::string(list) + "[" + std::to_string(index) + "]";
}

}  // namespace

void finish_map_checks(world::MapData& map, const world::Tileset& tileset, const std::string& file,
                       ErrorList& errors) {
    compute_walkability(map, tileset);  // 1) 타일로 지나갈 수 있는 칸 계산

    // 2) 등장 자리가 벽 위라면 캐릭터가 갇혀 버려요.
    for (std::size_t i = 0; i < map.spawn_points.size(); ++i) {
        if (!map.is_walkable(map.spawn_points[i].position)) {
            errors.add(file, item_path("spawn_points", i), "spawn point is on a blocked tile");
        }
    }

    // 3) 물건은 모두 지나갈 수 있는 칸에만 놓을 수 있어요 (몬스터가 벽 속에 갇히지 않게, 11a).
    //    먼저 모두 확인한 뒤에 단단한 물건 칸을 막아요. 그래야 순서와 상관없이 같은 결과가 나와요.
    std::vector<bool> placed_ok(map.entities.size(), true);  // 물건마다 검사를 통과했나
    for (std::size_t i = 0; i < map.entities.size(); ++i) {
        if (!map.is_walkable(map.entities[i].tile)) {  // 벽·물 위에 놓았으면 틀린 것
            errors.add(file, item_path("entities", i), "entity is on a blocked tile");
            placed_ok[i] = false;
        }
    }
    for (std::size_t i = 0; i < map.entities.size(); ++i) {
        const auto& entity = map.entities[i];
        if (!placed_ok[i] || !world::blocks_movement(entity.type)) continue;  // 막지 않는 물건은 그대로
        map.walkable[map.index_of(entity.tile)] = 0;  // 이제 이 칸은 못 지나가요
    }

    // 4) 출구 영역에 걸어 들어갈 수 있는 칸이 하나도 없으면, 그 출구는 절대 못 써요.
    for (std::size_t i = 0; i < map.exits.size(); ++i) {
        if (!exit_is_reachable(map, map.exits[i])) {
            errors.add(file, item_path("exits", i), "exit area has no walkable tile");
        }
    }
}

}  // namespace wind::content
