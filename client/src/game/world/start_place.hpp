#pragma once

// 이 파일은 무엇을 하나요?
// "맵의 어느 칸에서 시작할까"를 찾는 도우미 함수 모음이에요. (13b, 16c에서 field_assets에서 떼어 냄)
// 필드를 만들 때(load_field_assets)와, 쓰러진 순간 깨어날 자리를 저장할 때(wake_place, 16c) 같이 써요.
// 맵 자료만 보고 그림·SDL은 몰라요.

#include "game/world/cushion.hpp"

#include "wind/world/map.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::game {

// 시작할 칸. 쿠션에서 시작하면 그 쿠션 id도 있어요 (주인공이 앉은 채 시작해요)
struct StartPlace {
    world::TileCoord tile;
    std::optional<std::string> cushion;
};

// 맵의 쿠션 자리마다 쿠션 정보를 만들어요 (13b). 이름이 없으면 지역 이름을 써요.
[[nodiscard]] std::vector<Cushion> load_cushions(const world::MapData& map);

// 등장 자리 이름으로 시작 칸을 찾아요. 순서: 등장 자리 -> 쿠션 id(쉬기·부활) -> "default"면 첫 자리.
// 못 찾으면 없음 (맵 자료나 저장된 부활 자리가 틀렸어요)
[[nodiscard]] std::optional<StartPlace> find_start(const world::MapData& map, const std::vector<Cushion>& cushions,
                                                   std::string_view spawn_id);

}  // namespace wind::game
