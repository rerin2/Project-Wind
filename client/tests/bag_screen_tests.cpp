// 이 파일은 무엇을 하나요?
// 인벤토리 화면의 생각 부분(칸 고르기, 작은 메뉴, 명령)을 숙제 검사(테스트)해요. (14c)
#include "game/ui/bag_screen.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::BagCommand;
using wind::game::BagInput;
using wind::game::BagScreen;

// 물약(소비)과 젤리(재료)가 든 도감
wind::items::ItemCatalog make_catalog() {
    wind::items::ItemCatalog catalog;
    wind::items::ItemDef potion;
    potion.id = "potion";
    potion.max_stack = 20;
    potion.kind = wind::items::ItemKind::Consumable;
    potion.use = wind::items::UseEffect{50, 0};
    catalog.add(potion);
    wind::items::ItemDef jelly;
    jelly.id = "jelly";
    jelly.max_stack = 99;
    catalog.add(jelly);
    return catalog;
}

struct Fixture {
    wind::items::ItemCatalog catalog = make_catalog();
    wind::items::Inventory bag;
    BagScreen screen;
    Fixture() {
        bag.try_add(catalog, "jelly", 3);   // 0번 칸
        bag.try_add(catalog, "potion", 2);  // 1번 칸
        screen.open();
    }
    BagCommand press(BagInput input) { return screen.handle(input, bag, catalog); }
};

TEST(BagScreen, CursorMovesInSixColumnsAndStopsAtEdges) {
    Fixture f;
    EXPECT_EQ(f.screen.cursor(), 0u);
    f.press(BagInput::Left);  // 왼쪽 끝: 그대로
    f.press(BagInput::Up);    // 위쪽 끝: 그대로
    EXPECT_EQ(f.screen.cursor(), 0u);
    f.press(BagInput::Down);
    EXPECT_EQ(f.screen.cursor(), 6u);  // 한 줄 아래 = 6칸 뒤
    for (int i = 0; i < 10; ++i) f.press(BagInput::Right);
    EXPECT_EQ(f.screen.cursor(), 11u);  // 줄 끝에서 멈춰요 (다음 줄로 넘어가지 않아요)
    for (int i = 0; i < 10; ++i) f.press(BagInput::Down);
    EXPECT_EQ(f.screen.cursor(), 23u);  // 마지막 칸
}

TEST(BagScreen, EmptySlotHasNoMenu) {
    Fixture f;
    f.press(BagInput::Down);  // 6번 칸: 비어 있음
    EXPECT_EQ(f.press(BagInput::Confirm).kind, BagCommand::Kind::None);
    EXPECT_FALSE(f.screen.menu_open());
}

TEST(BagScreen, DropFromMenuReturnsTheSlot) {
    Fixture f;
    f.press(BagInput::Right);  // 1번 칸: 물약
    f.press(BagInput::Confirm);
    ASSERT_TRUE(f.screen.menu_open());
    // 14c에는 "사용"이 흐려서 처음 고른 것은 버리기예요
    EXPECT_FALSE(f.screen.action_enabled(BagScreen::Action::Use, f.bag, f.catalog));
    EXPECT_EQ(f.screen.menu_choice(), BagScreen::Action::Drop);
    const auto command = f.press(BagInput::Confirm);
    EXPECT_EQ(command.kind, BagCommand::Kind::Drop);
    EXPECT_EQ(command.slot, 1u);
    EXPECT_FALSE(f.screen.menu_open());
}

TEST(BagScreen, UseOnlyForConsumablesWhenAvailable) {
    Fixture f;
    f.screen.set_use_available(true);
    f.press(BagInput::Confirm);  // 0번 칸: 젤리 (재료)
    EXPECT_FALSE(f.screen.action_enabled(BagScreen::Action::Use, f.bag, f.catalog));
    EXPECT_EQ(f.screen.menu_choice(), BagScreen::Action::Drop);
    f.press(BagInput::Back);  // 메뉴만 닫혀요
    EXPECT_TRUE(f.screen.is_open());
    f.press(BagInput::Right);  // 물약
    f.press(BagInput::Confirm);
    EXPECT_EQ(f.screen.menu_choice(), BagScreen::Action::Use);
    const auto command = f.press(BagInput::Confirm);
    EXPECT_EQ(command.kind, BagCommand::Kind::Use);
    EXPECT_EQ(command.slot, 1u);
}

TEST(BagScreen, MenuSkipsDisabledChoicesAndCancelDoesNothing) {
    Fixture f;
    f.press(BagInput::Confirm);  // 젤리: 버리기
    f.press(BagInput::Down);     // 취소
    EXPECT_EQ(f.screen.menu_choice(), BagScreen::Action::Cancel);
    f.press(BagInput::Down);     // 사용은 흐려서 건너뛰고 버리기로
    EXPECT_EQ(f.screen.menu_choice(), BagScreen::Action::Drop);
    f.press(BagInput::Up);       // 위로도 사용을 건너뛰어요
    EXPECT_EQ(f.screen.menu_choice(), BagScreen::Action::Cancel);
    EXPECT_EQ(f.press(BagInput::Confirm).kind, BagCommand::Kind::None);
    EXPECT_FALSE(f.screen.menu_open());
}

TEST(BagScreen, SlotEmptiedWhileMenuOpenDoesNothing) {
    Fixture f;
    f.press(BagInput::Confirm);  // 젤리 메뉴
    ASSERT_TRUE(f.bag.take_at(0, 3).has_value());  // 그사이 칸이 비었어요
    EXPECT_EQ(f.press(BagInput::Confirm).kind, BagCommand::Kind::None);
}

TEST(BagScreen, SortAndBackCommands) {
    Fixture f;
    EXPECT_EQ(f.press(BagInput::Sort).kind, BagCommand::Kind::Sort);
    EXPECT_EQ(f.press(BagInput::Back).kind, BagCommand::Kind::Close);
    EXPECT_FALSE(f.screen.is_open());
    EXPECT_EQ(f.press(BagInput::Down).kind, BagCommand::Kind::None);  // 닫혀 있으면 아무것도 안 해요
    f.screen.open();
    EXPECT_EQ(f.screen.cursor(), 0u);  // 다시 열면 지난 자리 그대로
}

}  // namespace
