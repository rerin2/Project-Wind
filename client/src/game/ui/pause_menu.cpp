// 이 파일은 무엇을 하나요?
// 일시정지 메뉴의 탭 넘기기, 탭마다 커서, 시스템 항목 고르기, 가방 탭으로 키 넘기기를 하는 곳이에요.

#include "game/ui/pause_menu.hpp"

#include <algorithm>

namespace wind::game {
namespace {

// 메뉴 입력 → 가방 화면 입력 (Tab은 가방이 쓰지 않아요)
BagInput to_bag(PauseInput input) {
    switch (input) {
        case PauseInput::Up: return BagInput::Up;
        case PauseInput::Down: return BagInput::Down;
        case PauseInput::Left: return BagInput::Left;
        case PauseInput::Right: return BagInput::Right;
        case PauseInput::Confirm: return BagInput::Confirm;
        case PauseInput::Sort: return BagInput::Sort;
        case PauseInput::Back:
        case PauseInput::NextTab: break;
    }
    return BagInput::Back;
}

}  // namespace

void PauseMenu::open(MenuTab tab) {
    open_ = true;
    tab_ = tab;
    bag_.open();  // 가방 화면도 열린 상태로 (작은 메뉴는 닫힌 채, 커서는 그대로)
}

void PauseMenu::close() {
    open_ = false;
    bag_.close();
}

PauseCommand PauseMenu::handle(PauseInput input, const items::Inventory& bag, const items::ItemCatalog& catalog,
                               std::size_t gear_rows, std::size_t spell_rows) {
    if (!open_) return {};
    if (input == PauseInput::NextTab) {  // Tab: 다음 탭 (끝에서 처음으로). 가방의 작은 메뉴는 닫아요
        tab_ = static_cast<MenuTab>((static_cast<std::size_t>(tab_) + 1) % kMenuTabCount);
        bag_.close();
        bag_.open();
        return {};
    }
    if (tab_ == MenuTab::Bag) {
        if (input == PauseInput::Back && !bag_.menu_open()) {  // 작은 메뉴가 없으면 Esc = 메뉴 닫기
            close();
            return {PauseCommand::Kind::Close, {}};
        }
        const BagCommand command = bag_.handle(to_bag(input), bag, catalog);  // 나머지는 원래 가방 화면 규칙
        if (command.kind == BagCommand::Kind::None) return {};
        return {PauseCommand::Kind::Bag, command};
    }
    if (input == PauseInput::Back) {  // 다른 탭: Esc = 메뉴 닫기
        close();
        return {PauseCommand::Kind::Close, {}};
    }
    switch (tab_) {
        case MenuTab::Gear: move_row(gear_row_, input, gear_rows); return {};
        case MenuTab::Spells: move_row(spell_row_, input, spell_rows); return {};
        case MenuTab::System: return handle_system(input);
        case MenuTab::Stats:
        case MenuTab::Bag: break;  // 능력치는 고를 것이 없어요
    }
    return {};
}

void PauseMenu::move_row(std::size_t& row, PauseInput input, std::size_t rows) {
    if (rows == 0) {
        row = 0;
        return;
    }
    row = std::min(row, rows - 1);  // 줄 수가 줄었으면 (마술을 잊는 일은 없지만) 안으로
    if (input == PauseInput::Up && row > 0) --row;
    if (input == PauseInput::Down && row + 1 < rows) ++row;
}

// 위·아래는 고를 수 있는 항목만 지나가요. 확인은 고를 수 있는 항목만 실행해요.
PauseCommand PauseMenu::handle_system(PauseInput input) {
    const int step = input == PauseInput::Up ? -1 : (input == PauseInput::Down ? 1 : 0);
    if (step != 0) {
        auto index = static_cast<int>(system_item_);
        for (int next = index + step; next >= 0 && next < static_cast<int>(kSystemItemCount); next += step) {
            if (system_enabled(static_cast<SystemItem>(next))) {
                index = next;
                break;
            }
        }
        system_item_ = static_cast<SystemItem>(index);
        return {};
    }
    if (input == PauseInput::Confirm && system_enabled(system_item_) && system_item_ == SystemItem::Quit) {
        return {PauseCommand::Kind::Quit, {}};
    }
    return {};
}

}  // namespace wind::game
