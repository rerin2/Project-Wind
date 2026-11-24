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
    EXPECT_EQ(progress.fall("maps/m.json", {10.0f, 20.0f}), 12);  // floor(25 / 2)
    EXPECT_EQ(progress.runes(), 13);
    EXPECT_EQ(progress.stats(), before);   // 레벨·능력치는 잃지 않아요
}

TEST(HeroProgress, EarnStopsAtCap) {
    auto progress = make_progress(wind::progression::RuneWallet::kMax - 1);
    EXPECT_EQ(progress.earn(12), 1);
    EXPECT_EQ(progress.runes(), wind::progression::RuneWallet::kMax);
}

// ---------------------------------------------------------------- 13b: 쿠션 ----

TEST(HeroProgress, RestAtSetsRespawnAndRemembersDiscovery) {
    auto progress = make_progress(0);
    EXPECT_FALSE(progress.last_cushion().has_value());       // 처음엔 쉰 적이 없어요
    EXPECT_TRUE(progress.rest_at("maps/a.json", "a1"));       // 처음 찾음
    EXPECT_TRUE(progress.discovered("a1"));
    EXPECT_FALSE(progress.discovered("b1"));
    EXPECT_FALSE(progress.rest_at("maps/a.json", "a1"));      // 두 번째는 처음이 아니에요
    EXPECT_TRUE(progress.rest_at("maps/b.json", "b1"));
    ASSERT_TRUE(progress.last_cushion().has_value());
    EXPECT_EQ(*progress.last_cushion(), (wind::game::CushionRef{"maps/b.json", "b1"}));  // 마지막 쿠션
}

// ---------------------------------------------------------------- 14e: 룬 흔적 ----

namespace {

TEST(HeroProgress, FallingLeavesATraceWithTheLostRunes) {
    auto progress = make_progress(0);
    progress.earn(25);
    EXPECT_FALSE(progress.trace().has_value());
    EXPECT_EQ(progress.fall("maps/a.json", {10.0f, 20.0f}), 12);
    ASSERT_TRUE(progress.trace().has_value());
    EXPECT_EQ(progress.trace()->map, "maps/a.json");
    EXPECT_FLOAT_EQ(progress.trace()->at.x, 10.0f);
    EXPECT_EQ(progress.trace()->amount, 12);  // 잃은 양 그대로
    EXPECT_EQ(progress.last_vanished(), 0);
}

TEST(HeroProgress, NoTraceWhenNothingIsLost) {
    auto progress = make_progress(0);
    progress.earn(1);                         // floor(1 / 2) = 0
    EXPECT_EQ(progress.fall("maps/a.json", {}), 0);
    EXPECT_FALSE(progress.trace().has_value());
}

TEST(HeroProgress, RecoveringGivesAllBackOnlyOnce) {
    auto progress = make_progress(0);
    progress.earn(100);
    (void)progress.fall("maps/a.json", {});   // 50 잃음 → 50 남음
    EXPECT_EQ(progress.recover_trace(), 50);
    EXPECT_EQ(progress.runes(), 100);
    EXPECT_FALSE(progress.trace().has_value());
    EXPECT_EQ(progress.recover_trace(), 0);   // 두 번은 안 돼요
    EXPECT_EQ(progress.runes(), 100);
}

TEST(HeroProgress, FallingAgainBeforeRecoveringLosesTheOldTrace) {
    auto progress = make_progress(0);
    progress.earn(100);
    (void)progress.fall("maps/a.json", {1.0f, 1.0f});  // 흔적 50 (a), 지갑 50
    progress.earn(10);                                  // 지갑 60
    EXPECT_EQ(progress.fall("maps/b.json", {2.0f, 2.0f}), 30);
    ASSERT_TRUE(progress.trace().has_value());
    EXPECT_EQ(progress.trace()->map, "maps/b.json");   // 새 자리
    EXPECT_EQ(progress.trace()->amount, 30);           // 이전 50은 더해지지 않아요
    EXPECT_EQ(progress.last_vanished(), 50);           // "되찾지 못한 룬 50이 사라졌다"
    EXPECT_EQ(progress.runes(), 30);
}

TEST(HeroProgress, OldTraceVanishesEvenIfTheNewLossIsZero) {
    auto progress = make_progress(0);
    progress.earn(4);
    (void)progress.fall("maps/a.json", {});      // 2 잃음: 흔적 2, 지갑 2
    (void)progress.fall("maps/a.json", {});      // 1 잃음: 흔적 1 (이전 2는 사라짐), 지갑 1
    EXPECT_EQ(progress.last_vanished(), 2);
    EXPECT_EQ(progress.fall("maps/c.json", {}), 0);  // floor(1 / 2) = 0
    EXPECT_FALSE(progress.trace().has_value());      // 새 흔적은 없고 이전 흔적도 사라졌어요
    EXPECT_EQ(progress.last_vanished(), 1);
}

TEST(HeroProgress, RecoveryStopsAtTheWalletCap) {
    auto progress = make_progress(wind::progression::RuneWallet::kMax);
    (void)progress.fall("maps/a.json", {});  // 절반을 흔적으로
    const auto in_trace = progress.trace()->amount;
    progress.earn(in_trace + 5);             // 지갑이 다시 상한
    EXPECT_EQ(progress.recover_trace(), 0);  // 넘치는 만큼은 버려요 (정수 넘침 없이)
    EXPECT_EQ(progress.runes(), wind::progression::RuneWallet::kMax);
    EXPECT_FALSE(progress.trace().has_value());
}

}  // namespace

// 저장에서 되살리기 (16b): 맞으면 통째로, 틀리면 아무것도 안 바뀜
TEST(HeroProgress, RestoreReplacesStateOrNothing) {
    auto progress = make_progress(100);
    Stats stats;
    stats[StatId::Strength] = 25;
    const wind::game::RuneTrace trace{"maps/test_cave.json", {40.0f, 50.0f}, 30};
    const wind::game::CushionRef cushion{"maps/test_meadow.json", "meadow_cushion"};
    // 부활 쿠션이 찾은 목록에 없으면 거절
    EXPECT_FALSE(progress.restore(stats, 500, trace, cushion, {"other"}));
    stats[StatId::Vigor] = 61;  // 범위 밖
    EXPECT_FALSE(progress.restore(stats, 500, trace, cushion, {"meadow_cushion"}));
    EXPECT_EQ(progress.runes(), 100);
    EXPECT_EQ(progress.level(), 1);

    stats[StatId::Vigor] = 10;
    ASSERT_TRUE(progress.restore(stats, 500, trace, cushion, {"meadow_cushion", "cave_cushion"}));
    EXPECT_EQ(progress.runes(), 500);
    EXPECT_EQ(progress.stats()[StatId::Strength], 25);
    ASSERT_TRUE(progress.trace().has_value());
    EXPECT_EQ(progress.trace()->amount, 30);
    EXPECT_EQ(progress.last_cushion(), cushion);
    EXPECT_TRUE(progress.discovered("cave_cushion"));
    EXPECT_EQ(progress.discovered_ids().size(), 2u);
}
