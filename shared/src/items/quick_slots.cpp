// 이 파일은 무엇을 하나요?
// 단축 아이템 칸: 등록(옮기기·풀기), 비우기, 가방에서 그 아이템 찾기.

#include "wind/items/quick_slots.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <algorithm>

namespace wind::items {
namespace {

const std::string kEmpty;  // 빈 칸이 돌려줄 빈 글자

}  // namespace

QuickSlots::Result QuickSlots::assign(std::size_t slot, std::string_view item_id, const ItemCatalog& catalog) {
    if (slot >= kCount) return Result::BadSlot;
    const auto* def = catalog.find(item_id);
    if (def == nullptr || !def->use) return Result::NotUsable;  // 쓸 수 있는 아이템만 (개발자와 정함)
    if (items_[slot] == item_id) {                               // 같은 칸에 한 번 더: 등록 풀기
        items_[slot].clear();
        return Result::Cleared;
    }
    for (auto& other : items_) {
        if (other == item_id) other.clear();  // 다른 칸에 있었으면 거기서 빼고 (한 아이템은 한 칸에만)
    }
    items_[slot] = std::string(item_id);
    return Result::Assigned;
}

void QuickSlots::clear(std::size_t slot) {
    if (slot < kCount) items_[slot].clear();
}

const std::string& QuickSlots::at(std::size_t slot) const {
    return slot < kCount ? items_[slot] : kEmpty;
}

std::optional<std::size_t> QuickSlots::slot_of(std::string_view item_id) const {
    if (item_id.empty()) return std::nullopt;
    const auto it = std::ranges::find(items_, item_id);
    if (it == items_.end()) return std::nullopt;
    return static_cast<std::size_t>(it - items_.begin());
}

std::optional<std::size_t> QuickSlots::stack_for(std::size_t slot, const Inventory& bag) const {
    const std::string& id = at(slot);
    if (id.empty()) return std::nullopt;  // 빈 칸
    for (std::size_t i = 0; i < bag.slot_count(); ++i) {
        const auto& stack = bag.at(i);
        if (stack && stack->item_id == id) return i;  // 앞에서부터 첫 묶음
    }
    return std::nullopt;  // 다 써서 0개 (등록은 남아요)
}

}  // namespace wind::items
