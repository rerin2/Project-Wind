// 이 파일은 무엇을 하나요?
// 레벨업 계산 종이 검사 (13c, 명세 19장 단계 13 완료 조건)
// - [−][+]는 지금 능력치 ~ 60 안에서만 움직여요
// - 룬이 모자라면 확정이 안 되고 아무것도 바뀌지 않아요 (전부 성공 또는 전부 실패)
// - 힘을 올리면 힘 보너스가, 생명력을 올리면 최대 HP가 늘어요
#include "game/ui/level_up_sheet.hpp"

#include "wind/content/progression_loader.hpp"

#include <gtest/gtest.h>

using wind::game::HeroProgress;
using wind::game::LevelUpSheet;
using wind::progression::StatId;

namespace {

HeroProgress make_progress(std::int64_t runes) {
    auto rules = wind::content::load_progression_rules(WIND_TEST_CONTENT_DIR);
    EXPECT_TRUE(rules.ok());
    return HeroProgress(rules.ok() ? *rules.value : wind::progression::ProgressionRules{}, {}, runes);
}

}  // namespace

TEST(LevelUpSheet, ChangeStaysBetweenCurrentAndSixty) {
    const auto progress = make_progress(0);
    LevelUpSheet sheet;
    sheet.reset(progress);
    EXPECT_FALSE(sheet.change(-1, progress));  // 지금(10)보다 낮게는 안 돼요
    EXPECT_EQ(sheet.target()[StatId::Vigor], 10);
    EXPECT_TRUE(sheet.change(+60, progress));
    EXPECT_EQ(sheet.target()[StatId::Vigor], 60);  // 60에서 멈춰요
    EXPECT_FALSE(sheet.change(+1, progress));
}

TEST(LevelUpSheet, RowsWrapAndConfirmRowIgnoresChange) {
    const auto progress = make_progress(0);
    LevelUpSheet sheet;
    sheet.reset(progress);
    sheet.move(-1);                                 // 첫 줄에서 위로 -> 맨 아래 "확정"
    EXPECT_EQ(sheet.row(), LevelUpSheet::kConfirmRow);
    EXPECT_FALSE(sheet.change(1, progress));        // 확정 줄에서는 아무 일 없음
    sheet.move(1);
    EXPECT_EQ(sheet.row(), 0u);                     // 다시 첫 줄
}

TEST(LevelUpSheet, NotEnoughRunesChangesNothing) {
    auto progress = make_progress(99);  // 1→2는 100룬
    LevelUpSheet sheet;
    sheet.reset(progress);
    ASSERT_TRUE(sheet.change(1, progress));
    EXPECT_EQ(sheet.cost(progress), 100);
    EXPECT_FALSE(sheet.can_confirm(progress));
    EXPECT_FALSE(sheet.confirm(progress));
    EXPECT_EQ(progress.level(), 1);    // 능력치 그대로
    EXPECT_EQ(progress.runes(), 99);   // 룬 그대로
}

TEST(LevelUpSheet, ConfirmRaisesStrengthAndVigor) {
    auto progress = make_progress(10000);
    LevelUpSheet sheet;
    sheet.reset(progress);
    ASSERT_TRUE(sheet.change(2, progress));      // 생명력 +2 (0번 줄)
    sheet.move(3);                                // 힘 줄
    ASSERT_TRUE(sheet.change(10, progress));      // 힘 +10 -> 20
    EXPECT_EQ(sheet.target_level(), 13);
    ASSERT_TRUE(sheet.confirm(progress));
    EXPECT_EQ(progress.level(), 13);
    EXPECT_EQ(progress.derived().max_hp, 118);              // HP 바가 길어져요
    EXPECT_FLOAT_EQ(progress.derived().strength_bonus, 0.5f);  // 칼 공격력이 올라요
    EXPECT_FALSE(sheet.changed(progress));                   // 새 능력치로 다시 맞춰졌어요
}

TEST(LevelUpSheet, NothingChangedCannotConfirm) {
    auto progress = make_progress(10000);
    LevelUpSheet sheet;
    sheet.reset(progress);
    EXPECT_FALSE(sheet.can_confirm(progress));
    EXPECT_FALSE(sheet.confirm(progress));
    EXPECT_EQ(progress.runes(), 10000);
}
