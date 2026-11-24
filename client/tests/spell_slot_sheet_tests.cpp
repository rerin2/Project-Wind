// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 "마술 칸" 화면(15f)을 숙제 검사(테스트)해요: 진짜 마술 파일과 성장 규칙으로
// 칸 고르기(열린 칸만), 마술 넣기, 다른 칸에 있으면 맞바꾸기, 비우기, 지능이 모자라면 거절(아무것도 안 바뀜),
// 쿠션 메뉴에서 들어가고 나오기, 바뀌면 SpellsChanged.
#include "game/ui/spell_slot_sheet.hpp"

#include "game/combat/spell_shelf.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/ui/cushion_menu.hpp"

#include "wind/progression/stats.hpp"

#include <gtest/gtest.h>

#include <optional>

namespace {

using wind::game::MenuInput;
using wind::game::SpellSlotSheet;
using wind::progression::StatId;

// 지능 20 (칸 3개): 별빛 조각(칸 1), 달빛 칼날(칸 2), 혜성 광선(지능 36, 배우기만)
struct Fixture {
    Fixture() {
        wind::progression::Stats stats;
        stats[StatId::Intelligence] = 20;
        progress.emplace(*wind::game::HeroProgress::load(WIND_TEST_CONTENT_DIR, stats));
        for (const char* id : {"star_shard", "moon_blade", "comet_beam"}) {
            EXPECT_TRUE(shelf.load(WIND_TEST_CONTENT_DIR, id)) << id;
            progress->spells().learn(id);
        }
        auto& book = progress->spells();
        book.put(0, "star_shard", 10, 20, 3);
        book.put(1, "moon_blade", 14, 20, 3);
    }
    SpellSlotSheet::Result press(MenuInput input) { return sheet.handle(input, *progress, shelf); }
    [[nodiscard]] std::string_view slot(std::size_t i) const { return progress->spells().in_slot(i, 3); }

    wind::game::SpellShelf shelf;
    std::optional<wind::game::HeroProgress> progress;
    SpellSlotSheet sheet;
};

TEST(SpellSlotSheet, MovesOnlyThroughOpenSlots) {
    Fixture f;
    EXPECT_EQ(SpellSlotSheet::open_slots(*f.progress), 3u);
    for (int i = 0; i < 10; ++i) f.press(MenuInput::Down);
    EXPECT_EQ(f.sheet.slot(), 2u);  // 4번째 칸은 잠겨 있어요
    EXPECT_EQ(f.press(MenuInput::Back), SpellSlotSheet::Result::Back);
}

// 빈 3번 칸에 별빛 조각을 고르면: 1번 칸과 맞바꿔요 (1번이 비고 3번에 들어가요, 개발자와 정함)
TEST(SpellSlotSheet, PickingASlottedSpellSwaps) {
    Fixture f;
    f.press(MenuInput::Down);
    f.press(MenuInput::Down);   // 3번 칸 (빈 칸)
    f.press(MenuInput::Confirm);
    ASSERT_EQ(f.sheet.phase(), SpellSlotSheet::Phase::Picking);
    EXPECT_EQ(f.sheet.pick(), 0u);  // 빈 칸이면 목록 맨 위에서 시작
    EXPECT_EQ(f.press(MenuInput::Confirm), SpellSlotSheet::Result::Changed);  // 별빛 조각
    EXPECT_TRUE(f.slot(0).empty());
    EXPECT_EQ(f.slot(2), "star_shard");
    EXPECT_EQ(f.sheet.phase(), SpellSlotSheet::Phase::Slots);  // 칸 고르기로 돌아와요

    f.press(MenuInput::Up);  // 2번 칸 (달빛 칼날) ↔ 3번 칸 (별빛 조각)
    f.press(MenuInput::Confirm);
    EXPECT_EQ(f.sheet.pick(), 1u);  // 지금 든 마술(달빛 칼날) 줄에서 시작
    f.press(MenuInput::Up);         // 별빛 조각
    EXPECT_EQ(f.press(MenuInput::Confirm), SpellSlotSheet::Result::Changed);
    EXPECT_EQ(f.slot(1), "star_shard");
    EXPECT_EQ(f.slot(2), "moon_blade");
}

// 지능이 모자라면 거절되고 아무것도 안 바뀌어요. 고르는 단계에 그대로 남아요
TEST(SpellSlotSheet, NotEnoughIntelligenceIsDenied) {
    Fixture f;
    f.press(MenuInput::Confirm);  // 1번 칸
    f.press(MenuInput::Down);
    f.press(MenuInput::Down);     // 혜성 광선 (지능 36)
    EXPECT_EQ(f.press(MenuInput::Confirm), SpellSlotSheet::Result::Denied);
    EXPECT_EQ(f.slot(0), "star_shard");
    EXPECT_EQ(f.sheet.phase(), SpellSlotSheet::Phase::Picking);
    EXPECT_EQ(f.press(MenuInput::Back), SpellSlotSheet::Result::None);  // 뒤로: 칸 고르기로
    EXPECT_EQ(f.sheet.phase(), SpellSlotSheet::Phase::Slots);
}

TEST(SpellSlotSheet, LastRowClearsTheSlot) {
    Fixture f;
    f.press(MenuInput::Down);     // 2번 칸 (달빛 칼날)
    f.press(MenuInput::Confirm);
    for (int i = 0; i < 10; ++i) f.press(MenuInput::Down);
    EXPECT_EQ(f.sheet.pick(), 3u);  // 배운 마술 3개 + "비우기"
    EXPECT_EQ(f.press(MenuInput::Confirm), SpellSlotSheet::Result::Changed);
    EXPECT_TRUE(f.slot(1).empty());
}

// 쿠션 메뉴: 선반을 알려 주면 "마술 칸"이 켜지고, 들어가서 바꾸면 SpellsChanged, 뒤로면 메뉴로
TEST(SpellSlotSheet, CushionMenuOpensTheSheet) {
    Fixture f;
    wind::game::CushionMenu menu;
    menu.set_spell_shelf(f.shelf);
    EXPECT_TRUE(menu.item_enabled(wind::game::CushionMenu::Item::SpellSlots));
    menu.open();
    (void)menu.handle(MenuInput::Down, *f.progress);  // 레벨업 → 마술 칸
    ASSERT_EQ(menu.focused_item(), wind::game::CushionMenu::Item::SpellSlots);
    (void)menu.handle(MenuInput::Confirm, *f.progress);
    ASSERT_EQ(menu.screen(), wind::game::CushionMenu::Screen::SpellSlots);
    (void)menu.handle(MenuInput::Confirm, *f.progress);  // 1번 칸 → 목록
    (void)menu.handle(MenuInput::Down, *f.progress);     // 달빛 칼날
    EXPECT_EQ(menu.handle(MenuInput::Confirm, *f.progress), wind::game::MenuResult::SpellsChanged);
    EXPECT_EQ(f.slot(0), "moon_blade");
    EXPECT_EQ(f.slot(1), "star_shard");
    const unsigned denied = menu.denied_count();
    (void)menu.handle(MenuInput::Confirm, *f.progress);  // 1번 칸 → 목록 (지금 든 달빛 칼날 줄)
    (void)menu.handle(MenuInput::Down, *f.progress);     // 혜성 광선
    EXPECT_EQ(menu.handle(MenuInput::Confirm, *f.progress), wind::game::MenuResult::Denied);
    EXPECT_EQ(menu.denied_count(), denied + 1);           // 화면이 흔들려요
    (void)menu.handle(MenuInput::Back, *f.progress);      // 목록 → 칸
    (void)menu.handle(MenuInput::Back, *f.progress);      // 칸 → 메뉴
    EXPECT_EQ(menu.screen(), wind::game::CushionMenu::Screen::Main);
}

}  // namespace
