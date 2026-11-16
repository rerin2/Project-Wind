#pragma once

// 이 파일은 무엇을 하나요?
// 맵을 다 읽은 뒤에 하는 "마무리 검사" 도우미예요. (map_loader.cpp에서 나눠 왔어요)
// 1) 칸마다 지나갈 수 있는지 계산해요 (타일 카드 성질로).
// 2) 등장 자리는 지나갈 수 있는 칸이어야 해요.
// 3) 단단한 물건(허수아비)은 지나갈 수 있는 칸에만 놓고, 놓은 칸은 막아요.
// 4) 출구 영역에는 걸어 들어갈 수 있는 칸이 하나 이상 있어야 해요.
// 틀린 곳은 errors에 "어느 파일의 어느 칸"으로 적어요.

#include "wind/content/error_list.hpp"
#include "wind/world/map.hpp"

#include <string>

namespace wind::content {

// map: 레이어·등장 자리·출구·물건까지 읽은 맵. 지나갈 수 있는 칸 표(walkable)를 채워요.
void finish_map_checks(world::MapData& map, const world::Tileset& tileset, const std::string& file,
                       ErrorList& errors);

}  // namespace wind::content
