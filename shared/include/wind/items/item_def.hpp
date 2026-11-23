#pragma once

// 이 파일은 무엇을 하나요?
// 아이템 하나의 "설계도"(정의)예요. (명세 10.1, 14a)
// 예: "회복 물약은 HP를 50 채우고, 한 칸에 20개까지 쌓여요."
// 실제로 가방에 든 물건(몇 개 있는지)은 ItemStack이 따로 적어요.
// 숫자는 content/items/catalog.json에서 읽어요. 게임 규칙이라 SDL을 몰라요.

#include <cstdint>
#include <optional>
#include <string>

namespace wind::items {

// 아이템 종류
enum class ItemKind {
    Consumable,  // 써서 없어지는 것 (물약)
    Material,    // 재료 (몬스터 젤리·날개, 18단계 상점·퀘스트에서 써요)
    Weapon,      // 무기 (장착은 15단계)
};

// 희귀도: 인벤토리 칸 테두리 색이 달라져요 (14c)
enum class Rarity {
    Common,
    Uncommon,
    Rare,
};

// 장비 칸 (명세 10.3). 14단계에서는 무기만 써요.
enum class EquipSlot {
    Weapon,
    Head,
    Body,
    Accessory,
};

// 쓰면 생기는 일 (소비 아이템만). 0이면 그 효과가 없어요.
struct UseEffect {
    std::int32_t heal_hp = 0;  // HP를 이만큼 채워요
    std::int32_t heal_mp = 0;  // MP를 이만큼 채워요
};

struct ItemDef {
    std::string id;                    // 게임 전체에서 하나뿐인 이름 ("hp_potion")
    std::string name;                  // 화면에 보일 이름 ("회복 물약")
    std::string description;           // 설명 한두 줄
    ItemKind kind = ItemKind::Material;
    std::int32_t max_stack = 1;        // 한 칸에 쌓이는 최대 개수 (1이면 안 쌓여요)
    std::int32_t icon = 0;             // 아이콘 그림 판의 몇 번째 칸인지 (0부터)
    Rarity rarity = Rarity::Common;
    std::optional<UseEffect> use;      // 소비 아이템이면 있어요
    std::optional<EquipSlot> equip;    // 장비면 있어요
    std::string weapon;                // 무기면 무기 파일 위치 ("weapons/longsword.json"), 아니면 빈 글자
};

}  // namespace wind::items
