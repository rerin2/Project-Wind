#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 지금 끼고 있는 장비예요. (명세 10.2, 10.3, 15a)
// - 칸마다 아이템 id 하나 (비었으면 빈 글자). 15단계에서는 무기 칸만 써요.
// - 장착(equip_from): 가방 한 칸의 장비를 꺼내 끼고, 끼고 있던 것은 꺼낸 그 칸에 넣어요(바꿔 끼우기).
//   꺼낸 칸이 늘 비게 되니 가방이 가득 차도 바꿀 수 있어요. 실패하면 아무것도 안 바뀌어요(전부 성공 또는 전부 실패).
// - 벗기(unequip): 가방에 빈 자리가 있을 때만 넣고 칸을 비워요.
// 장비는 가방 밖에 있어서 가방 칸을 차지하지 않아요. 저장 파일에는 칸마다 id를 적어요(16단계).
// 게임 규칙이라 SDL을 몰라요.

#include "wind/items/equip_slot.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace wind::items {

class Inventory;
class ItemCatalog;

class Equipment {
public:
    // 장착·벗기 결과 (실패면 아무것도 안 바뀌었어요)
    enum class Result : std::uint8_t {
        Done,          // 바뀌었어요
        EmptySlot,     // 그 가방 칸이 비었거나 번호가 밖이에요
        NotEquipment,  // 도감에 없거나 장비가 아니에요 (물약·재료)
        NothingWorn,   // 벗을 것이 없어요
        BagFull,       // 벗은 것을 넣을 자리가 없어요
    };

    // 이 칸에 끼고 있는 아이템 id (비었으면 빈 글자)
    [[nodiscard]] const std::string& worn(EquipSlot slot) const { return worn_[index(slot)]; }
    // 처음 상태를 정해요 (시작 장비, 저장 불러오기). 가방은 건드리지 않아요.
    void wear(EquipSlot slot, std::string item_id) { worn_[index(slot)] = std::move(item_id); }

    // 가방 bag_slot의 장비를 끼어요. 그 칸에 맞는 장비 칸에 있던 것은 bag_slot으로 돌아가요.
    Result equip_from(Inventory& bag, std::size_t bag_slot, const ItemCatalog& catalog);
    // 이 칸의 장비를 벗어 가방에 넣어요.
    Result unequip(EquipSlot slot, Inventory& bag, const ItemCatalog& catalog);

private:
    [[nodiscard]] static std::size_t index(EquipSlot slot) { return static_cast<std::size_t>(slot); }

    std::array<std::string, kEquipSlotCount> worn_;  // 칸마다 끼고 있는 아이템 id
};

}  // namespace wind::items
