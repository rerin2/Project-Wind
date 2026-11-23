#pragma once

// 이 파일은 무엇을 하나요?
// 실제로 가진 물건 한 묶음이에요 (명세 10.1의 "인스턴스"). 예: 회복 물약 3개.
// 인벤토리 칸 하나, 바닥에 떨어진 아이템 하나가 이 모양이에요.
// 개별 속성(강화 수치 같은 것)은 확장 버전에서 칸을 더해요.

#include <cstdint>
#include <string>

namespace wind::items {

struct ItemStack {
    std::string item_id;     // 어떤 아이템인지 (ItemDef::id)
    std::int32_t count = 0;  // 몇 개인지 (1 ~ max_stack)

    friend bool operator==(const ItemStack&, const ItemStack&) = default;
};

}  // namespace wind::items
