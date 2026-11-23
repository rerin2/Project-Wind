// 이 파일은 무엇을 하나요?
// 드롭 표 굴리기: 줄마다 "나오나?" 한 번, 나오면 "몇 개?" 한 번.

#include "game/items/drop_roll.hpp"

#include "game/combat/combat_random.hpp"

#include "wind/items/item_catalog.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

std::vector<items::ItemStack> roll_drops(const std::vector<items::DropEntry>& table, CombatRandom& random) {
    std::vector<items::ItemStack> out;
    for (const auto& line : table) {
        if (!random.roll(line.chance)) continue;  // 이번에는 안 나왔어요
        const std::int32_t low = std::min(line.min_count, line.max_count);
        const std::int32_t high = std::max(line.min_count, line.max_count);
        const std::int32_t choices = high - low + 1;  // 나올 수 있는 개수 가짓수
        // 0~1 미만 수 × 가짓수 → 0 ~ 가짓수-1 (끝값이 넘지 않게 한 번 더 잘라요)
        const auto pick = static_cast<std::int32_t>(std::floor(random.next_unit() * static_cast<float>(choices)));
        out.push_back({line.item_id, low + std::clamp(pick, 0, choices - 1)});
    }
    return out;
}

std::optional<std::string> unknown_drop_item(const std::vector<items::DropEntry>& table,
                                             const items::ItemCatalog& catalog) {
    for (const auto& line : table) {
        if (catalog.find(line.item_id) == nullptr) return line.item_id;  // 도감에 없는 아이템
    }
    return std::nullopt;
}

}  // namespace wind::game
