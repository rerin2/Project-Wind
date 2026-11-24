// 이 파일은 무엇을 하나요?
// 일시정지 메뉴의 생각 부분(15e)을 숙제 검사(테스트)해요:
// 탭 넘기기(끝에서 처음으로), 가방 탭은 원래 인벤토리 규칙 그대로, Esc는 작은 메뉴 먼저 닫고 그다음 메뉴를 닫음,
// 장비·마술 줄 고르기(끝에서 멈춤), 시스템 탭은 고를 수 있는 "게임 끄기"만, 열려 있으면 게임 입력을 막음.
#include "game/ui/pause_menu.hpp"

#include "wind/items/inventory.hpp"
#include "wind/items/item_catalog.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::BagCommand;
using wind::game::MenuTab;
using wind::game::PauseCommand;
using wind::game::PauseInput;
using wind::game::PauseMenu;
using wind::game::SystemItem;

constexpr std::size_t kGearRows = 4;
constexpr std::size_t kSpellRows = 3;

struct Fixture {
    wind::items::ItemCatalog catalog;
    wind::items::Inventory bag;
    PauseMenu menu;
    Fixture() {
        wind::items::ItemDef potion;
        potion.id = "potion";
        potion.max_stack = 20;
        potion.kind = wind::items::ItemKind::Consumable;
        potion.use = wind::items::UseEffect{50, 0};
        catalog.add(potion);
        bag.try_add(catalog, "potion", 2);  // 0번 칸
        menu.set_use_available(true);
    }
    PauseCommand press(PauseInput input) { return menu.handle(input, bag, catalog, kGearRows, kSpellRows); }
};

// 완료 기준: 메뉴가 열리면 게임 입력이 막혀요 (닫으면 풀려요)
TEST(PauseMenu, CapturesInputOnlyWhileOpen) {
    Fixture f;
    EXPECT_FALSE(f.menu.captures_input());
    f.menu.open();
    EXPECT_TRUE(f.menu.captures_input());
    EXPECT_EQ(f.press(PauseInput::Back).kind, PauseCommand::Kind::Close);
    EXPECT_FALSE(f.menu.is_open());
    EXPECT_FALSE(f.menu.captures_input());
}

TEST(PauseMenu, TabCyclesThroughFiveTabsAndReopensOnTheLastOne) {
    Fixture f;
    f.menu.open(MenuTab::Bag);
    const MenuTab order[] = {MenuTab::Gear, MenuTab::Spells, MenuTab::Stats, MenuTab::System, MenuTab::Bag};
    for (const MenuTab expected : order) {
        EXPECT_EQ(f.press(PauseInput::NextTab).kind, PauseCommand::Kind::None);
        EXPECT_EQ(f.menu.tab(), expected);
    }
    f.press(PauseInput::NextTab);  // 장비
    f.menu.close();
    f.menu.open();  // Esc/Tab: 마지막으로 보던 탭
    EXPECT_EQ(f.menu.tab(), MenuTab::Gear);
    f.menu.open(MenuTab::Bag);  // I: 바로 가방 탭
    EXPECT_EQ(f.menu.tab(), MenuTab::Bag);
}

// 가방 탭은 원래 인벤토리 규칙 그대로: 칸에서 Enter → 작은 메뉴 → "사용"이면 Use 명령
TEST(PauseMenu, BagTabForwardsToTheBagScreen) {
    Fixture f;
    f.menu.open(MenuTab::Bag);
    EXPECT_EQ(f.press(PauseInput::Confirm).kind, PauseCommand::Kind::None);  // 작은 메뉴가 열려요
    ASSERT_TRUE(f.menu.bag_screen().menu_open());
    const auto command = f.press(PauseInput::Confirm);  // 첫 항목 "사용"
    ASSERT_EQ(command.kind, PauseCommand::Kind::Bag);
    EXPECT_EQ(command.bag.kind, BagCommand::Kind::Use);
    EXPECT_EQ(command.bag.slot, 0u);
    EXPECT_EQ(f.press(PauseInput::Sort).bag.kind, BagCommand::Kind::Sort);  // R: 정렬
}

// Esc: 가방의 작은 메뉴가 열려 있으면 그것만 닫고, 다음 Esc에 메뉴를 닫아요
TEST(PauseMenu, BackClosesTheSmallBagMenuFirst) {
    Fixture f;
    f.menu.open(MenuTab::Bag);
    f.press(PauseInput::Confirm);
    ASSERT_TRUE(f.menu.bag_screen().menu_open());
    EXPECT_EQ(f.press(PauseInput::Back).kind, PauseCommand::Kind::None);
    EXPECT_FALSE(f.menu.bag_screen().menu_open());
    EXPECT_TRUE(f.menu.is_open());
    EXPECT_EQ(f.press(PauseInput::Back).kind, PauseCommand::Kind::Close);
    EXPECT_FALSE(f.menu.is_open());
}

// Tab으로 탭을 바꾸면 가방의 작은 메뉴는 닫혀요 (돌아왔을 때 열린 채 남지 않게)
TEST(PauseMenu, SwitchingTabsClosesTheSmallBagMenu) {
    Fixture f;
    f.menu.open(MenuTab::Bag);
    f.press(PauseInput::Confirm);
    f.press(PauseInput::NextTab);
    EXPECT_FALSE(f.menu.bag_screen().menu_open());
    EXPECT_TRUE(f.menu.bag_screen().is_open());
}

TEST(PauseMenu, GearAndSpellRowsStopAtTheEnds) {
    Fixture f;
    f.menu.open(MenuTab::Gear);
    f.press(PauseInput::Up);
    EXPECT_EQ(f.menu.gear_row(), 0u);
    for (int i = 0; i < 10; ++i) f.press(PauseInput::Down);
    EXPECT_EQ(f.menu.gear_row(), kGearRows - 1);
    f.press(PauseInput::NextTab);  // 마술
    for (int i = 0; i < 10; ++i) f.press(PauseInput::Down);
    EXPECT_EQ(f.menu.spell_row(), kSpellRows - 1);
    EXPECT_EQ(f.press(PauseInput::Confirm).kind, PauseCommand::Kind::None);  // 보기만 (아무 일도 없어요)
}

// 시스템 탭: 설정·타이틀로는 흐려서 고를 수 없고, "게임 끄기"를 고르면 Quit 명령
TEST(PauseMenu, SystemTabOnlyQuitIsEnabled) {
    Fixture f;
    f.menu.open(MenuTab::System);
    EXPECT_FALSE(PauseMenu::system_enabled(SystemItem::Settings));
    EXPECT_FALSE(PauseMenu::system_enabled(SystemItem::Title));
    EXPECT_EQ(f.menu.system_item(), SystemItem::Quit);
    f.press(PauseInput::Up);  // 위는 흐린 항목뿐: 그대로
    EXPECT_EQ(f.menu.system_item(), SystemItem::Quit);
    EXPECT_EQ(f.press(PauseInput::Confirm).kind, PauseCommand::Kind::Quit);
}

TEST(PauseMenu, ClosedMenuIgnoresInput) {
    Fixture f;
    EXPECT_EQ(f.press(PauseInput::Confirm).kind, PauseCommand::Kind::None);
    EXPECT_EQ(f.press(PauseInput::NextTab).kind, PauseCommand::Kind::None);
    EXPECT_FALSE(f.menu.is_open());
}

}  // namespace
