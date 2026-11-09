// 이 파일은 무엇을 하나요?
// map.hpp에서 약속한 "맵에게 물어보기" 기능(이 칸 갈 수 있어? 여기 무슨 카드야?)을 실제로 해요.

#include "wind/world/map.hpp"

#include <algorithm>

namespace wind::world {

// 카드 목록을 처음부터 끝까지 훑으며 번호가 같은 카드를 찾아요.
const TileProperties* Tileset::find(TileId tile) const {
    const auto it = std::ranges::find(tiles, tile, &TileProperties::id);
    return it != tiles.end() ? &*it : nullptr;  // 끝까지 못 찾으면 빈손
}

// x와 y가 둘 다 0 이상이고, 가로·세로 칸 수보다 작으면 바둑판 안이에요.
bool MapData::in_bounds(TileCoord pos) const {
    return pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height;
}

// 줄 우선 순서: 위에 있는 줄들의 칸 수(y * width)를 건너뛴 다음, 그 줄에서 x번째
std::size_t MapData::index_of(TileCoord pos) const {
    return static_cast<std::size_t>(pos.y) * static_cast<std::size_t>(width) +
           static_cast<std::size_t>(pos.x);
}

// 먼저 바둑판 안인지 확인해요. 밖이면 표를 들여다보지 않아요 (엉뚱한 기억을 읽지 않게).
bool MapData::is_walkable(TileCoord pos) const {
    return in_bounds(pos) && walkable[index_of(pos)] != 0;
}

TileId MapData::tile_at(const MapLayer& layer, TileCoord pos) const {
    return in_bounds(pos) ? layer.tiles[index_of(pos)] : kEmptyTile;
}

// 등장 자리 목록에서 이름이 같은 것을 찾아요.
const MapExit* MapData::exit_at(TileCoord tile) const {
    for (const MapExit& exit : exits) {
        if (exit.contains(tile)) {
            return &exit;
        }
    }
    return nullptr;
}

const SpawnPoint* MapData::find_spawn(std::string_view spawn_id) const {
    const auto it = std::ranges::find(spawn_points, spawn_id, &SpawnPoint::id);
    return it != spawn_points.end() ? &*it : nullptr;
}

}  // namespace wind::world
