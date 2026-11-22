#pragma once

// 이 파일은 무엇을 하나요?
// 맵에 놓인 캣닢 쿠션(세이브 포인트) 하나의 정보예요. (명세 8.7, 13b)
// 맵의 "save_point" 물건에서 만들어요. 그냥 값 묶음이라 SDL을 몰라요.

#include "engine/core/math.hpp"

#include "wind/world/direction.hpp"

#include <string>

namespace wind::game {

struct Cushion {
    std::string id;           // 게임 전체에서 하나뿐인 이름 (부활 자리·찾은 쿠션 기록에 써요)
    std::string name;         // 배너에 보여 줄 이름 (맵 파일의 display_name, 없으면 지역 이름)
    world::TileCoord tile;    // 놓인 칸
    engine::Vec2 seat;        // 주인공이 앉는 자리 = 칸 가운데 아래쪽의 발 위치 (픽셀)
};

}  // namespace wind::game
