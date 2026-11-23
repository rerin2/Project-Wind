#pragma once

// 이 파일은 무엇을 하나요?
// 장비 칸 이름이에요. (명세 10.3) 아이템 정의(ItemDef)와 장비(Equipment)가 같이 써요.
// 15단계에서는 무기 칸만 써요. 머리·몸·장신구는 확장 버전에서 채워요.

#include <cstddef>

namespace wind::items {

enum class EquipSlot {
    Weapon,
    Head,
    Body,
    Accessory,
};

inline constexpr std::size_t kEquipSlotCount = 4;  // 위 칸 수

}  // namespace wind::items
