// 이 파일은 무엇을 하나요?
// 쿠션 메뉴: 항목 고르기, 레벨업 화면 오가기, 확정과 떠나기.

#include "game/ui/cushion_menu.hpp"

#include "game/progression/hero_progress.hpp"

namespace wind::game {

CushionMenu::CushionMenu() : focus_(kItemCount) {
    // 아직 없는 기능은 흐리게(고를 수 없게): 저장은 16단계 (13c, 개발자와 정함).
    // 마술 칸은 마술 선반을 알려 줄 때 켜져요 (15f)
    focus_.set_enabled(static_cast<std::size_t>(Item::SpellSlots), false);
    focus_.set_enabled(static_cast<std::size_t>(Item::Save), false);
}

void CushionMenu::set_spell_shelf(const SpellShelf& shelf) {
    shelf_ = &shelf;
    focus_.set_enabled(static_cast<std::size_t>(Item::SpellSlots), true);
}

void CushionMenu::open() {
    screen_ = Screen::Main;
    while (focus_.current() != static_cast<std::size_t>(Item::LevelUp)) focus_.move(-1);  // 늘 "레벨업"부터
}

MenuResult CushionMenu::handle(MenuInput input, HeroProgress& progress) {
    switch (screen_) {
        case Screen::Closed: return MenuResult::None;
        case Screen::Main: return handle_main(input, progress);
        case Screen::LevelUp: return handle_level_up(input, progress);
        case Screen::SpellSlots: return handle_spell_slots(input, progress);
    }
    return MenuResult::None;
}

MenuResult CushionMenu::handle_main(MenuInput input, HeroProgress& progress) {
    switch (input) {
        case MenuInput::Up: focus_.move(-1); return MenuResult::None;   // 흐린 항목은 건너뛰어요
        case MenuInput::Down: focus_.move(1); return MenuResult::None;
        case MenuInput::Back:                                           // Esc = 떠나기
            screen_ = Screen::Closed;
            return MenuResult::Leave;
        case MenuInput::Confirm:
            if (focused_item() == Item::LevelUp) {
                sheet_.reset(progress);                                 // 지금 능력치로 계산 종이 준비
                screen_ = Screen::LevelUp;
                return MenuResult::None;
            }
            if (focused_item() == Item::SpellSlots && shelf_ != nullptr) {  // 마술 칸 화면으로 (15f)
                slot_sheet_.reset();
                screen_ = Screen::SpellSlots;
                return MenuResult::None;
            }
            if (focused_item() == Item::Leave) {
                screen_ = Screen::Closed;
                return MenuResult::Leave;
            }
            return MenuResult::None;                                    // 흐린 항목은 고를 수 없어요
        case MenuInput::Left:
        case MenuInput::Right:
            return MenuResult::None;
    }
    return MenuResult::None;
}

MenuResult CushionMenu::handle_level_up(MenuInput input, HeroProgress& progress) {
    switch (input) {
        case MenuInput::Up: sheet_.move(-1); return MenuResult::None;
        case MenuInput::Down: sheet_.move(1); return MenuResult::None;
        case MenuInput::Left: (void)sheet_.change(-1, progress); return MenuResult::None;   // [−]
        case MenuInput::Right: (void)sheet_.change(1, progress); return MenuResult::None;   // [+]
        case MenuInput::Back:                                           // 바꾼 건 버리고 메뉴로
            screen_ = Screen::Main;
            return MenuResult::None;
        case MenuInput::Confirm:
            if (sheet_.row() != LevelUpSheet::kConfirmRow) {            // 능력치 줄에서 확인 = [+]
                (void)sheet_.change(1, progress);
                return MenuResult::None;
            }
            if (sheet_.confirm(progress)) return MenuResult::LeveledUp;  // 전부 성공
            ++denied_count_;                                             // 전부 실패: 아무것도 안 바뀌었어요
            return MenuResult::Denied;
    }
    return MenuResult::None;
}

MenuResult CushionMenu::handle_spell_slots(MenuInput input, HeroProgress& progress) {
    if (shelf_ == nullptr) {  // 선반이 없으면 들어올 수 없지만, 혹시 몰라 메뉴로
        screen_ = Screen::Main;
        return MenuResult::None;
    }
    switch (slot_sheet_.handle(input, progress, *shelf_)) {
        case SpellSlotSheet::Result::Changed: return MenuResult::SpellsChanged;
        case SpellSlotSheet::Result::Denied:
            ++denied_count_;  // 지능이 모자라요: 화면이 흔들려요 (레벨업 거절과 같은 횟수)
            return MenuResult::Denied;
        case SpellSlotSheet::Result::Back:
            screen_ = Screen::Main;
            return MenuResult::None;
        case SpellSlotSheet::Result::None: break;
    }
    return MenuResult::None;
}

}  // namespace wind::game
