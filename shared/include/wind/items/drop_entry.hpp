#pragma once

// 이 파일은 무엇을 하나요?
// 드롭 표의 한 줄이에요. (명세 9.7, 14b) 예: "슬라임 젤리가 50% 확률로 1~2개 나와요."
// 몬스터 데이터의 "drops" 목록이 이 줄들로 되어 있고, 줄마다 따로 주사위를 굴려요.

#include <cstdint>
#include <string>

namespace wind::items {

struct DropEntry {
    std::string item_id;        // 나오는 아이템 (도감 id)
    float chance = 1.0f;        // 나올 확률 (0보다 크고 1 이하)
    std::int32_t min_count = 1; // 나오면 최소 몇 개
    std::int32_t max_count = 1; // 나오면 최대 몇 개
};

}  // namespace wind::items
