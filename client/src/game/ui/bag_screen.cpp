// 이 파일은 무엇을 하나요?
// 인벤토리 화면: 칸 고르기, 작은 메뉴(사용·버리기·취소), 정렬·닫기 명령.

#include "game/ui/bag_screen.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

namespace wind::game {

void BagScreen::open() {
    open_ = true;
    menu_open_ = false;
}

void BagScreen::close() {
    open_ = false;
    menu_open_ = false;
}

bool BagScreen::action_enabled(Action action, const items::Inventory& bag, const items::ItemCatalog& catalog) const {
    const auto& stack = bag.at(cursor_);
    switch (action) {
        case Action::Use: {
            if (!stack) return false;
            const auto* def = catalog.find(stack->item_id);
            if (def == nullptr) return false;
            if (def->equip) return true;                          // 장비는 "장착" (15a)
            return use_available_ && def->use.has_value();        // 소비 아이템은 "사용"
        }
        case Action::Drop: return stack.has_value();  // 무엇이든 내려놓을 수 있어요 (도감에 없는 것도)
        case Action::Cancel: return true;
    }
    return false;
}

BagCommand BagScreen::handle(BagInput input, const items::Inventory& bag, const items::ItemCatalog& catalog) {
    if (!open_) return {};
    if (menu_open_) return handle_menu(input, bag, catalog);
    switch (input) {
        case BagInput::Up:
        case BagInput::Down:
        case BagInput::Left:
        case BagInput::Right:
            move_cursor(input, bag.slot_count());
            return {};
        case BagInput::Confirm:
            if (!bag.at(cursor_)) return {};  // 빈 칸은 메뉴가 없어요
            menu_open_ = true;
            choice_ = Action::Cancel;
            move_choice(+1, bag, catalog);  // 취소 다음(맨 앞)부터 고를 수 있는 첫 항목
            return {};
        case BagInput::Sort:
            return {BagCommand::Kind::Sort, 0};
        case BagInput::Back:
            close();
            return {BagCommand::Kind::Close, 0};
    }
    return {};
}

void BagScreen::move_cursor(BagInput input, std::size_t slot_count) {
    if (slot_count == 0) return;
    const std::size_t column = cursor_ % kColumns;
    // 끝에서는 멈춰요 (넘어가서 반대편으로 돌지 않아요)
    if (input == BagInput::Left && column > 0) --cursor_;
    if (input == BagInput::Right && column + 1 < kColumns && cursor_ + 1 < slot_count) ++cursor_;
    if (input == BagInput::Up && cursor_ >= kColumns) cursor_ -= kColumns;
    if (input == BagInput::Down && cursor_ + kColumns < slot_count) cursor_ += kColumns;
}

void BagScreen::move_choice(int step, const items::Inventory& bag, const items::ItemCatalog& catalog) {
    // 항목 수만큼 돌면 반드시 고를 수 있는 것(취소)을 만나요
    auto index = static_cast<int>(choice_);
    for (std::size_t tries = 0; tries < kActionCount; ++tries) {
        index = (index + step + static_cast<int>(kActionCount)) % static_cast<int>(kActionCount);
        if (action_enabled(static_cast<Action>(index), bag, catalog)) break;
    }
    choice_ = static_cast<Action>(index);
}

BagCommand BagScreen::handle_menu(BagInput input, const items::Inventory& bag, const items::ItemCatalog& catalog) {
    switch (input) {
        case BagInput::Up:
        case BagInput::Left:
            move_choice(-1, bag, catalog);
            return {};
        case BagInput::Down:
        case BagInput::Right:
            move_choice(+1, bag, catalog);
            return {};
        case BagInput::Back:
        case BagInput::Sort:  // 메뉴가 열려 있을 때 정렬은 메뉴만 닫아요 (고른 칸이 움직이지 않게)
            menu_open_ = false;
            return {};
        case BagInput::Confirm: break;
    }
    menu_open_ = false;  // 무엇을 고르든 메뉴는 닫혀요
    if (!action_enabled(choice_, bag, catalog)) return {};  // 그사이 칸이 비었으면 아무 일도 없어요
    switch (choice_) {
        case Action::Use: {  // 장비면 장착, 소비 아이템이면 사용 (15a)
            const auto* def = catalog.find(bag.at(cursor_)->item_id);  // action_enabled가 칸·도감을 확인했어요
            return {def->equip ? BagCommand::Kind::Equip : BagCommand::Kind::Use, cursor_};
        }
        case Action::Drop: return {BagCommand::Kind::Drop, cursor_};
        case Action::Cancel: break;
    }
    return {};
}

}  // namespace wind::game
