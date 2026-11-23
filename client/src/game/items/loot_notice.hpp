#pragma once

// 이 파일은 무엇을 하나요?
// 화면 왼쪽 아래 알림 줄에 띄울 일 하나예요. (14b, 개발자와 정함; 14d에 물약 알림 두 가지, 15a에 장착 알림 세 가지를 더함)
// 필드가 모아 두고 앱이 가져가요.

#include "wind/items/item_stack.hpp"

#include <cstdint>

namespace wind::game {

struct LootNotice {
    enum class Kind : std::uint8_t {
        PickedUp,     // "슬라임 젤리 ×2" (주움)
        BagFull,      // "가방이 가득 찼다 · 이름" (못 주움)
        CannotUse,    // "지금은 마실 수 없다" (공격·구르기·맞는 중 등, 14d)
        AlreadyFull,  // "이미 가득하다 · 이름" (채울 HP·MP가 없어 안 마심, 14d)
        Equipped,       // "단검 장착" (무기를 바꿔 낌, 15a)
        EquippedHeavy,  // "대검 장착 · 너무 무겁다" (무거움, 15a)
        CannotEquip,    // "지금은 바꿀 수 없다" (공격·구르기·맞는 중, 15a)
    };
    items::ItemStack stack;     // 무엇이 몇 개
    Kind kind = Kind::PickedUp;

    // 빨간 경고 줄인가요? (주운 것과 가벼운 무기 장착만 크림색)
    [[nodiscard]] bool warning() const { return kind != Kind::PickedUp && kind != Kind::Equipped; }
};

}  // namespace wind::game
