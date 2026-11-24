// 이 파일은 무엇을 하나요?
// 마술 공책: 배우기, 칸에 넣기(요구 지능 확인), Q로 고르기.

#include "wind/progression/spell_book.hpp"

#include <algorithm>
#include <utility>

namespace wind::progression {

std::size_t SpellBook::open_count(std::size_t open_slots) {
    return std::min(open_slots, kMaxSlots);  // 곡선이 4보다 크게 줘도 4칸까지만
}

bool SpellBook::learn(std::string id) {
    if (id.empty() || knows(id)) return false;
    learned_.push_back(std::move(id));
    return true;
}

bool SpellBook::knows(std::string_view id) const {
    return std::ranges::find(learned_, id) != learned_.end();
}

SpellBook::SlotResult SpellBook::put(std::size_t slot, std::string_view id, int required, int intelligence,
                                     std::size_t open_slots) {
    if (slot >= open_count(open_slots)) return SlotResult::LockedSlot;
    if (!knows(id)) return SlotResult::NotLearned;
    if (intelligence < required) return SlotResult::NeedsIntelligence;  // 지능이 모자라요 (명세 8.6)
    for (std::size_t i = 0; i < kMaxSlots; ++i) {
        if (i != slot && slots_[i] == id) return SlotResult::AlreadySlotted;
    }
    slots_[slot] = std::string(id);
    return SlotResult::Done;
}

SpellBook::SlotResult SpellBook::put_in_free_slot(std::string_view id, int required, int intelligence,
                                                  std::size_t open_slots) {
    for (std::size_t i = 0; i < open_count(open_slots); ++i) {
        if (slots_[i].empty()) return put(i, id, required, intelligence, open_slots);
    }
    return SlotResult::LockedSlot;  // 열린 칸이 모두 찼어요
}

SpellBook::SlotResult SpellBook::place(std::size_t slot, std::string_view id, int required, int intelligence,
                                       std::size_t open_slots) {
    if (slot >= open_count(open_slots)) return SlotResult::LockedSlot;     // 잠긴 칸
    if (!knows(id)) return SlotResult::NotLearned;                         // 배우지 않은 마술
    if (intelligence < required) return SlotResult::NeedsIntelligence;     // 지능이 모자라요 (명세 8.6)
    const auto other = std::ranges::find(slots_, id);                      // 다른 칸에 이미 있나
    if (other != slots_.end()) {
        std::swap(*other, slots_[slot]);  // 맞바꾸기: 이 칸의 마술(또는 빈칸)이 그 칸으로 가요
        return SlotResult::Done;
    }
    slots_[slot] = std::string(id);
    return SlotResult::Done;
}

void SpellBook::clear(std::size_t slot) {
    if (slot < kMaxSlots) slots_[slot].clear();
}

std::string_view SpellBook::in_slot(std::size_t slot, std::size_t open_slots) const {
    if (slot >= open_count(open_slots)) return {};  // 잠긴 칸
    return slots_[slot];
}

std::size_t SpellBook::selected_slot(std::size_t open_slots) const {
    if (!in_slot(selected_, open_slots).empty()) return selected_;
    for (std::size_t i = 0; i < open_count(open_slots); ++i) {  // 고른 칸이 비었으면 맨 앞 마술
        if (!slots_[i].empty()) return i;
    }
    return 0;
}

std::string_view SpellBook::selected(std::size_t open_slots) const {
    return in_slot(selected_slot(open_slots), open_slots);
}

bool SpellBook::select_next(std::size_t open_slots) {
    const std::size_t count = open_count(open_slots);
    const std::size_t from = selected_slot(open_slots);
    for (std::size_t step = 1; step < count; ++step) {
        const std::size_t next = (from + step) % count;  // 끝이면 처음으로 돌아가요
        if (!slots_[next].empty()) {
            selected_ = next;
            return true;
        }
    }
    selected_ = from;  // 다른 마술이 없어요
    return false;
}

}  // namespace wind::progression
