// 이 파일은 무엇을 하나요?
// 주인공 진행 상태 검사 (13a)
// - 진짜 성장 규칙 파일로 만들 수 있나, 시작은 레벨 1·룬 0·HP 100인가
// - 레벨업 확정은 전부 성공 또는 전부 실패 (룬이 모자라면 아무것도 안 바뀜, 명세 19장 단계 13)
// - 능력치를 내리거나 60을 넘기는 목표는 거절하나
// - 쓰러지면 룬이 절반(소수점 버림)으로 줄고 능력치는 그대로인가
#include "game/progression/hero_progress.hpp"

#include "wind/content/progression_loader.hpp"

#include <gtest/gtest.h>

using wind::game::HeroProgress;
using wind::progression::StatId;
using wind::progression::Stats;

namespace {

// 진짜 규칙 파일 + 처음 룬
HeroProgress make_progress(std::int64_t runes) {
    auto rules = wind::content::load_progression_rules(WIND_TEST_CONTENT_DIR);
    EXPECT_TRUE(rules.ok());
    return HeroProgress(rules.ok() ? *rules.value : wind::progression::ProgressionRules{}, Stats{}, runes);
}

// 지금 능력치에서 한 가지만 amount만큼 올린 목표
Stats raised(const HeroProgress& progress, StatId id, int amount) {
    Stats target = progress.stats();
    target[id] += amount;
    return target;
}

}  // namespace

TEST(HeroProgress, LoadsRealRulesAndStartsAtLevelOne) {
    const auto progress = HeroProgress::load(WIND_TEST_CONTENT_DIR);
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->level(), 1);
    EXPECT_EQ(progress->runes(), 0);
    EXPECT_EQ(progress->derived().max_hp, 100);
}

TEST(HeroProgress, LoadFailsOnMissingFolder) {
    EXPECT_FALSE(HeroProgress::load("this/folder/does/not/exist").has_value());
}

TEST(HeroProgress, RaisingVigorLengthensHp) {
    auto progress = make_progress(1000);
    const Stats target = raised(progress, StatId::Vigor, 2);  // 레벨 1 → 3: 100 + 146 = 246룬
    EXPECT_EQ(progress.cost_to(target), 246);
    EXPECT_EQ(progress.preview(target).max_hp, 118);           // 미리 보기 (아직 안 바뀜)
    EXPECT_EQ(progress.derived().max_hp, 100);
    ASSERT_TRUE(progress.try_raise(target));
    EXPECT_EQ(progress.level(), 3);
    EXPECT_EQ(progress.runes(), 1000 - 246);
    EXPECT_EQ(progress.derived().max_hp, 118);
}

TEST(HeroProgress, NotEnoughRunesChangesNothing) {
    auto progress = make_progress(245);                     // 1 모자라요
    const Stats before = progress.stats();
    EXPECT_FALSE(progress.try_raise(raised(progress, StatId::Strength, 2)));
    EXPECT_EQ(progress.stats(), before);                    // 능력치 그대로
    EXPECT_EQ(progress.runes(), 245);                       // 룬도 그대로
    EXPECT_EQ(progress.level(), 1);
}

TEST(HeroProgress, CannotLowerOrPassSixty) {
    auto progress = make_progress(100000000);
    EXPECT_FALSE(progress.cost_to(raised(progress, StatId::Mind, -1)).has_value());  // 내리기
    EXPECT_FALSE(progress.try_raise(raised(progress, StatId::Mind, -1)));
    EXPECT_FALSE(progress.cost_to(raised(progress, StatId::Mind, 51)).has_value());  // 61
    EXPECT_TRUE(progress.try_raise(raised(progress, StatId::Mind, 50)));             // 60은 돼요
    EXPECT_EQ(progress.stats()[StatId::Mind], 60);
}

TEST(HeroProgress, SameStatsCostNothing) {
    auto progress = make_progress(0);
    EXPECT_EQ(progress.cost_to(progress.stats()), 0);
    EXPECT_TRUE(progress.try_raise(progress.stats()));  // 바뀌는 것 없이 성공
}

TEST(HeroProgress, FallingLosesHalfRoundedDown) {
    auto progress = make_progress(0);
    progress.earn(12);
    progress.earn(13);                     // 25
    const Stats before = progress.stats();
    EXPECT_EQ(progress.fall(), 12);        // floor(25 / 2)
    EXPECT_EQ(progress.runes(), 13);
    EXPECT_EQ(progress.stats(), before);   // 레벨·능력치는 잃지 않아요
}

TEST(HeroProgress, EarnStopsAtCap) {
    auto progress = make_progress(wind::progression::RuneWallet::kMax - 1);
    EXPECT_EQ(progress.earn(12), 1);
    EXPECT_EQ(progress.runes(), wind::progression::RuneWallet::kMax);
}
