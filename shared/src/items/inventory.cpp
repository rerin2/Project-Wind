// 이 파일은 무엇을 하나요?
// 가방 규칙: 넣기·빼기·꺼내기·정렬. 모두 "먼저 되는지 확인하고, 될 때만 바꾸기" 순서예요.

#include "wind/items/inventory.hpp"

#include "wind/items/item_catalog.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <utility>

namespace wind::items {
namespace {

// 칸 번호가 밖일 때 돌려줄 "빈 칸" (참조로 돌려주려고 하나만 둬요, 바뀌지 않는 값)
const std::optional<ItemStack> kNoStack;

}  // namespace

Inventory::Inventory(std::size_t slots) : slots_(slots) {}

const std::optional<ItemStack>& Inventory::at(std::size_t slot) const {
    return slot < slots_.size() ? slots_[slot] : kNoStack;
}

std::int64_t Inventory::count_of(std::string_view item_id) const {
    std::int64_t total = 0;
    for (const auto& s : slots_) {
        if (s && s->item_id == item_id) total += s->count;
    }
    return total;
}

std::size_t Inventory::free_slots() const {
    // 비어 있는 칸(값이 없는 칸)만 세요
    return static_cast<std::size_t>(
        std::ranges::count_if(slots_, [](const auto& s) { return !s.has_value(); }));
}

std::int64_t Inventory::room_for(std::string_view item_id, std::int32_t max_stack) const {
    std::int64_t room = 0;
    for (const auto& s : slots_) {
        if (!s) {
            room += max_stack;  // 빈 칸은 한 묶음을 통째로 받아요
        } else if (s->item_id == item_id) {
            room += std::max<std::int64_t>(0, max_stack - s->count);  // 같은 아이템 칸의 남은 자리
        }
    }
    return room;
}

bool Inventory::can_add(const ItemCatalog& catalog, std::string_view item_id,
                        std::int32_t count) const {
    const ItemDef* def = catalog.find(item_id);
    if (def == nullptr || count < 1) return false;  // 모르는 아이템이거나 이상한 개수
    return room_for(item_id, def->max_stack) >= count;
}

bool Inventory::try_add(const ItemCatalog& catalog, std::string_view item_id, std::int32_t count) {
    if (!can_add(catalog, item_id, count)) return false;  // 먼저 확인: 안 되면 아무것도 안 바꿔요
    const std::int32_t max_stack = catalog.find(item_id)->max_stack;
    std::int32_t left = count;  // 아직 못 넣은 개수
    // 1) 같은 아이템 칸부터 채워요
    for (auto& s : slots_) {
        if (left == 0) break;
        if (!s || s->item_id != item_id) continue;
        const std::int32_t put = std::min(left, std::max(0, max_stack - s->count));
        s->count += put;
        left -= put;
    }
    // 2) 남으면 앞쪽 빈 칸부터 새 묶음을 놓아요
    for (auto& s : slots_) {
        if (left == 0) break;
        if (s) continue;
        const std::int32_t put = std::min(left, max_stack);
        s = ItemStack{std::string(item_id), put};
        left -= put;
    }
    return true;  // can_add로 확인했으니 left는 0이에요
}

bool Inventory::try_remove(std::string_view item_id, std::int32_t count) {
    if (count < 1 || count_of(item_id) < count) return false;  // 모자라면 그대로
    std::int32_t left = count;  // 아직 못 뺀 개수
    // 뒤쪽 칸부터 줄여요 (앞쪽 묶음이 남아 있게)
    for (auto it = slots_.rbegin(); it != slots_.rend() && left > 0; ++it) {
        auto& s = *it;
        if (!s || s->item_id != item_id) continue;
        const std::int32_t take = std::min(left, s->count);
        s->count -= take;
        left -= take;
        if (s->count == 0) s.reset();  // 다 쓴 칸은 비워요
    }
    return true;
}

std::optional<ItemStack> Inventory::take_at(std::size_t slot, std::int32_t count) {
    if (slot >= slots_.size() || !slots_[slot] || count < 1) return std::nullopt;
    auto& s = slots_[slot];
    if (count > s->count) return std::nullopt;  // 가진 것보다 많이는 못 꺼내요
    ItemStack taken{s->item_id, count};
    s->count -= count;
    if (s->count == 0) s.reset();
    return taken;
}

bool Inventory::place_at(std::size_t slot, ItemStack stack) {
    if (slot >= slots_.size() || slots_[slot] || stack.count < 1) return false;  // 빈 칸에만
    slots_[slot] = std::move(stack);
    return true;
}

bool Inventory::sort(const ItemCatalog& catalog) {
    // 1) 도감에 있는 아이템은 id별로 모두 더하고, 없는 아이템은 그대로 따로 모아요
    std::map<std::size_t, std::pair<std::string, std::int64_t>> known;  // 도감 순서 → (id, 합계)
    std::vector<ItemStack> unknown;
    for (const auto& s : slots_) {
        if (!s) continue;
        if (catalog.find(s->item_id) == nullptr) {
            unknown.push_back(*s);
            continue;
        }
        auto& entry = known[catalog.rank(s->item_id)];
        entry.first = s->item_id;
        entry.second += s->count;
    }
    // 2) 새 칸을 따로 만들어요: 도감 순서대로 max_stack씩 나눠 놓고, 모르는 아이템은 뒤에
    std::vector<std::optional<ItemStack>> sorted;
    for (const auto& [rank, entry] : known) {
        const std::int64_t max_stack = catalog.find(entry.first)->max_stack;
        for (std::int64_t left = entry.second; left > 0; left -= max_stack) {
            sorted.emplace_back(ItemStack{entry.first, static_cast<std::int32_t>(std::min(left, max_stack))});
        }
    }
    for (auto& s : unknown) sorted.emplace_back(std::move(s));
    // 3) 칸이 모자라면(이상한 저장 파일 등) 아무것도 바꾸지 않아요
    if (sorted.size() > slots_.size()) return false;
    sorted.resize(slots_.size());  // 나머지는 빈 칸
    slots_ = std::move(sorted);
    return true;
}

}  // namespace wind::items
