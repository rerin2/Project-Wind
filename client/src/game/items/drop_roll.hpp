#pragma once

// 이 파일은 무엇을 하나요?
// 드롭 표로 주사위를 굴려 "실제로 나온 아이템"을 정해요. (명세 9.7, 14b)
// - 줄마다 따로: chance 확률로 나오고, 나오면 min~max개 중 하나 (모두 같은 확률)
// - 싸움 주사위(CombatRandom)를 써서 시드가 같으면 결과도 같아요 (테스트·재현).
// 게임 규칙이라 SDL을 몰라요.

#include "wind/items/drop_entry.hpp"
#include "wind/items/item_stack.hpp"

#include <optional>
#include <string>
#include <vector>

namespace wind::items {
class ItemCatalog;
}

namespace wind::game {

class CombatRandom;

// 나온 아이템 묶음들 (나온 줄 순서대로, 안 나온 줄은 빠져요)
[[nodiscard]] std::vector<items::ItemStack> roll_drops(const std::vector<items::DropEntry>& table,
                                                       CombatRandom& random);

// 드롭 표에 도감에 없는 아이템이 있으면 그 id를 돌려줘요 (없으면 nullopt).
// 필드를 열 때 확인해서, 틀린 몬스터 데이터가 있는 맵은 열지 않아요 (명세 14.4 참조 검사).
[[nodiscard]] std::optional<std::string> unknown_drop_item(const std::vector<items::DropEntry>& table,
                                                           const items::ItemCatalog& catalog);

}  // namespace wind::game
