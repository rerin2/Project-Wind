#pragma once

// 이 파일은 무엇을 하나요?
// 룬 흔적 하나예요. (명세 8.3.1, 14e, 결정 0010)
// 쓰러진 자리에 잃은 룬이 남은 것이에요. 게임 전체에 최대 하나이고, HeroProgress가 가지고 있어요
// (맵을 옮기거나 부활해도 남고, 16단계 저장에도 들어가요).

#include "engine/core/math.hpp"

#include <cstdint>
#include <string>

namespace wind::game {

struct RuneTrace {
    std::string map;          // 흔적이 있는 맵 파일 (콘텐츠 폴더 기준, 예: "maps/test_meadow.json")
    engine::Vec2 at;          // 쓰러진 자리 (주인공 발 위치, 맵 위 픽셀)
    std::int64_t amount = 0;  // 담긴 룬 (쓰러질 때 잃은 양)
};

}  // namespace wind::game
