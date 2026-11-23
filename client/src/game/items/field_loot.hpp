#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 "아이템 떨어뜨리기와 줍기" 규칙이에요. (명세 9.7, 10.2, 14b)
// 정해진 박자마다:
//   1) 이번 박자에 쓰러진 몬스터마다 드롭 표를 굴려, 나온 아이템을 둘레에 흩어 놓아요.
//      (드롭 주사위는 싸움 주사위와 따로라서, 줍기 때문에 몬스터 걸음이 바뀌지 않아요)
//   2) 바닥 아이템이 나이를 먹어요. 떨어지고 0.6초(튀어 오르는 시간) 안에는 못 주워요.
//   3) 주인공 발이 36픽셀 안에 오면 가방에 넣어요 (전부 들어갈 때만). 넣으면 바닥에서 지워져
//      다시는 주워지지 않아요. 가득 차서 못 넣으면 바닥에 두고 한 번만 알려요.
//      48픽셀 밖으로 멀어졌다 다시 오면 또 알려요.
//   쓰러졌거나 쉬는 중이면 줍지 않아요.
// 바닥 목록(MapLoot)은 필드 밖(LootStore)에 있어서 맵을 떠나도 남아요. 게임 규칙이라 SDL을 몰라요.

#include "game/combat/combat_random.hpp"
#include "game/items/ground_item.hpp"

#include "wind/items/item_stack.hpp"

#include <cstdint>
#include <vector>

namespace wind::world {
struct MapData;
}
namespace wind::items {
class Inventory;
class ItemCatalog;
}  // namespace wind::items

namespace wind::game {

class FieldCombat;
class MapLoot;
class PlayerMotion;

class FieldLoot {
public:
    static constexpr float kSettleSeconds = 0.6f;  // 떨어진 뒤 이만큼은 못 주워요 (튀어 오르는 연출 시간)
    static constexpr float kPickupRadius = 36.0f;  // 주인공 발에서 이 안이면 주워요 (픽셀, 약 2칸)
    static constexpr float kReleaseRadius = 48.0f; // 이보다 멀어지면 "가득 찼다"를 다시 알릴 수 있어요

    // loot: 이 맵의 바닥 목록 (필드보다 오래 살아요), seed: 드롭 주사위 씨앗
    FieldLoot(MapLoot& loot, std::uint64_t seed);

    // 한 박자: 드롭 -> 나이 먹기 -> 줍기
    void step(float dt_seconds, const FieldCombat& combat, const PlayerMotion& hero,
              const world::MapData& map, items::Inventory& bag, const items::ItemCatalog& catalog);

    [[nodiscard]] const MapLoot& loot() const { return loot_; }
    // 이번 박자에 주운 아이템들 (다음 박자에 지워져요, 연출·알림용)
    [[nodiscard]] const std::vector<GroundItem>& picked() const { return picked_; }
    // 이번 박자에 가방이 가득 차서 못 주운 아이템들 (다음 박자에 지워져요, 알림용)
    [[nodiscard]] const std::vector<items::ItemStack>& refused() const { return refused_; }

private:
    // 쓰러진 몬스터 자리 둘레에 드롭을 흩어 놓아요
    void spawn_drops(const FieldCombat& combat, const world::MapData& map);
    // 주인공 가까이 있는 아이템을 주워요
    void pick_up(const PlayerMotion& hero, items::Inventory& bag, const items::ItemCatalog& catalog);

    MapLoot& loot_;                          // 이 맵의 바닥 아이템 (LootStore가 가지고 있어요)
    CombatRandom random_;                    // 드롭 주사위 (몇 개, 어디에)
    std::vector<GroundItem> picked_;         // 이번 박자에 주운 것
    std::vector<items::ItemStack> refused_;  // 이번 박자에 가득 차서 못 주운 것
};

}  // namespace wind::game
