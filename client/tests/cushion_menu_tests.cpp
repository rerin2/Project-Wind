// 이 파일은 무엇을 하나요?
// 쿠션 메뉴 검사 (13c): 흐린 항목 건너뛰기, 레벨업 화면 오가기, 확정·거절, 떠나기.
#include "game/ui/cushion_menu.hpp"

#include "game/progression/hero_progress.hpp"

#include "wind/content/progression_loader.hpp"

#include <gtest/gtest.h>

using wind::game::CushionMenu;
using wind::game::HeroProgress;
using wind::game::MenuInput;
using wind::game::MenuResult;

namespace {

HeroProgress make_progress(std::int64_t runes) {
    auto rules = wind::content::load_progression_rules(WIND_TEST_CONTENT_DIR);
    EXPECT_TRUE(rules.ok());
    return HeroProgress(rules.ok() ? *rules.value : wind::progression::ProgressionRules{}, {}, runes);
}

}  // namespace

TEST(CushionMenu, OpensOnLevelUpAndSkipsDisabledItems) {
    auto progress = make_progress(0);
    CushionMenu menu;
    EXPECT_FALSE(menu.is_open());
    menu.open();
    EXPECT_EQ(menu.focused_item(), CushionMenu::Item::LevelUp);
    EXPECT_FALSE(menu.item_enabled(CushionMenu::Item::SpellSlots));  // 마술 선반을 알려 주기 전 (15f)
    EXPECT_FALSE(menu.item_enabled(CushionMenu::Item::Save));        // 16단계
    (void)menu.handle(MenuInput::Down, progress);
    EXPECT_EQ(menu.focused_item(), CushionMenu::Item::Leave);        // 흐린 두 항목은 건너뛰어요
}

TEST(CushionMenu, LeaveAndBackCloseTheMenu) {
    auto progress = make_progress(0);
    CushionMenu menu;
    menu.open();
    EXPECT_EQ(menu.handle(MenuInput::Back, progress), MenuResult::Leave);
    EXPECT_FALSE(menu.is_open());
    menu.open();
    (void)menu.handle(MenuInput::Down, progress);  // 떠나기
    EXPECT_EQ(menu.handle(MenuInput::Confirm, progress), MenuResult::Leave);
    EXPECT_FALSE(menu.is_open());
}

TEST(CushionMenu, LevelUpScreenConfirmsOrDenies) {
    auto progress = make_progress(150);
    CushionMenu menu;
    menu.open();
    (void)menu.handle(MenuInput::Confirm, progress);                // 레벨업 화면으로
    ASSERT_EQ(menu.screen(), CushionMenu::Screen::LevelUp);
    (void)menu.handle(MenuInput::Right, progress);                  // 생명력 +1 (100룬)
    (void)menu.handle(MenuInput::Right, progress);                  // 생명력 +2 (246룬: 모자람)
    (void)menu.handle(MenuInput::Up, progress);                     // 맨 아래 "확정"으로
    EXPECT_EQ(menu.handle(MenuInput::Confirm, progress), MenuResult::Denied);
    EXPECT_EQ(menu.denied_count(), 1u);
    EXPECT_EQ(progress.level(), 1);                                 // 아무것도 안 바뀌었어요
    (void)menu.handle(MenuInput::Down, progress);                   // 생명력 줄로
    (void)menu.handle(MenuInput::Left, progress);                   // +1로 줄이고
    (void)menu.handle(MenuInput::Up, progress);
    EXPECT_EQ(menu.handle(MenuInput::Confirm, progress), MenuResult::LeveledUp);
    EXPECT_EQ(progress.level(), 2);
    EXPECT_EQ(progress.runes(), 50);
    (void)menu.handle(MenuInput::Back, progress);                   // 메뉴로 돌아가요 (닫히지 않아요)
    EXPECT_EQ(menu.screen(), CushionMenu::Screen::Main);
}

TEST(CushionMenu, ClosedMenuIgnoresInput) {
    auto progress = make_progress(0);
    CushionMenu menu;
    EXPECT_EQ(menu.handle(MenuInput::Confirm, progress), MenuResult::None);
}
