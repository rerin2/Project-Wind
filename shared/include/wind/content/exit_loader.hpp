#pragma once

// 이 파일은 무엇을 하나요?
// 맵 파일의 "exits"(다른 맵으로 가는 출구) 목록을 읽고 숙제 검사를 하는 도우미예요. (명세 7.4)
//   "exits": [ { "area": { "x": 0, "y": 12, "w": 1, "h": 1 },
//                "to_map": "maps/test_cave.json", "to_spawn": "entrance" } ]
// 영역은 맵 안이어야 하고, 갈 맵 위치는 콘텐츠 폴더 안이어야 해요.
// 갈 맵과 등장 자리가 진짜 있는지는 실제로 옮겨 갈 때 확인해요.

#include "wind/content/error_list.hpp"
#include "wind/world/map.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>

namespace wind::content {

inline constexpr std::size_t kMaxMapExits = 32;

// list: exits 목록. map의 width/height는 먼저 채워져 있어야 해요. 읽은 출구는 map.exits에 넣어요.
void parse_exits(const nlohmann::json& list, const std::string& file, world::MapData& map,
                 ErrorList& errors);

}  // namespace wind::content
