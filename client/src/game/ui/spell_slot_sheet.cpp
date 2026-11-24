// 이 파일은 무엇을 하나요?
// 쿠션 "마술 칸" 화면: 칸 고르기 → 마술 고르기(또는 비우기) → 공책에 놓기(맞바꾸기).

#include "game/ui/spell_slot_sheet.hpp"

#include "game/combat/spell_shelf.hpp"
#include "game/progression/hero_progress.hpp"

#include "wind/progression/spell_book.hpp"

#include <algorithm>

namespace wind::game {
namespace {

// 아주 높은 요구 지능 (선반에 없는 마술은 넣을 수 없게)
constexpr int kUnknownRequirement = progression::kStatMax + 1;

}  // namespace

std::size_t SpellSlotSheet::open_slots(const HeroProgress& progress) {
    const auto slots = static_cast<std::size_t>(std::max(0, progress.derived().spell_slots));
    return std::min(slots, progression::SpellBook::kMaxSlots);
}

void SpellSlotSheet::reset() {
    phase_ = Phase::Slots;
    slot_ = 0;
    pick_ = 0;
}

SpellSlotSheet::Result SpellSlotSheet::handle(MenuInput input, HeroProgress& progress, const SpellShelf& shelf) {
    return phase_ == Phase::Slots ? handle_slots(input, progress) : handle_picking(input, progress, shelf);
}

// 1단계: 열린 칸 사이를 위·아래로 (끝에서 멈춰요)
SpellSlotSheet::Result SpellSlotSheet::handle_slots(MenuInput input, const HeroProgress& progress) {
    const std::size_t open = open_slots(progress);
    switch (input) {
        case MenuInput::Up:
            if (slot_ > 0) --slot_;
            return Result::None;
        case MenuInput::Down:
            if (slot_ + 1 < open) ++slot_;
            return Result::None;
        case MenuInput::Back: return Result::Back;
        case MenuInput::Confirm: {
            if (slot_ >= open) return Result::None;  // 열린 칸이 없어요
            // 2단계는 지금 이 칸에 든 마술 줄에서 시작해요 (비었으면 맨 위)
            const auto& learned = progress.spells().learned();
            const auto current = progress.spells().in_slot(slot_, open);
            const auto it = std::ranges::find(learned, current);
            pick_ = it != learned.end() ? static_cast<std::size_t>(it - learned.begin()) : 0;
            phase_ = Phase::Picking;
            return Result::None;
        }
        case MenuInput::Left:
        case MenuInput::Right: return Result::None;
    }
    return Result::None;
}

// 2단계: 배운 마술 + "비우기" 중에서 하나
SpellSlotSheet::Result SpellSlotSheet::handle_picking(MenuInput input, HeroProgress& progress, const SpellShelf& shelf) {
    auto& book = progress.spells();
    const std::size_t rows = book.learned().size() + 1;  // 마지막 줄 = "비우기"
    switch (input) {
        case MenuInput::Up:
            if (pick_ > 0) --pick_;
            return Result::None;
        case MenuInput::Down:
            if (pick_ + 1 < rows) ++pick_;
            return Result::None;
        case MenuInput::Back:
            phase_ = Phase::Slots;  // 칸 고르기로
            return Result::None;
        case MenuInput::Left:
        case MenuInput::Right: return Result::None;
        case MenuInput::Confirm: break;
    }
    const std::size_t open = open_slots(progress);
    if (pick_ + 1 == rows) {  // 비우기
        book.clear(slot_);
        phase_ = Phase::Slots;
        return Result::Changed;
    }
    const std::string& id = book.learned()[pick_];
    const auto* spell = shelf.find(id);
    const int required = spell != nullptr ? spell->intelligence_required : kUnknownRequirement;
    const int intelligence = progress.stats()[progression::StatId::Intelligence];
    if (book.place(slot_, id, required, intelligence, open) != progression::SpellBook::SlotResult::Done) {
        return Result::Denied;  // 지능이 모자라요: 아무것도 안 바뀌어요
    }
    phase_ = Phase::Slots;
    return Result::Changed;
}

}  // namespace wind::game
