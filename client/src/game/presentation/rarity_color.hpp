#pragma once

// 이 파일은 무엇을 하나요?
// 희귀도(보통·고급·희귀)마다 쓰는 색이에요. (명세 10.1 "희귀도 색 테두리", 14b·14c)
// 바닥 아이템의 밤 빛, 인벤토리 칸 테두리, 아이템 이름 글자가 같은 색을 써서 한눈에 알아봐요.
// 값의 이유는 docs/VISUALS.md "Ground items"·"Bag screen" 절에 있어요.

#include "engine/render/render_types.hpp"

#include "wind/items/item_def.hpp"

namespace wind::game {

// 희귀도 색 (보통: 따뜻한 흰색, 고급: 하늘색, 희귀: 분홍빛 보라)
[[nodiscard]] constexpr engine::Color rarity_color(items::Rarity rarity) {
    switch (rarity) {
        case items::Rarity::Uncommon: return {120, 220, 255, 255};
        case items::Rarity::Rare: return {255, 170, 255, 255};
        case items::Rarity::Common: break;
    }
    return {255, 240, 200, 255};
}

}  // namespace wind::game
