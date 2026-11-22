// 이 파일은 무엇을 하나요?
// 파생값 계산 검사 (13a): 진짜 곡선 데이터로 시작값·경계값(1, 60)·소수점 버림을 확인해요.
#include "wind/content/progression_loader.hpp"
#include "wind/progression/derived_stats.hpp"

#include <gtest/gtest.h>

using wind::progression::StatId;
using wind::progression::Stats;

namespace {

wind::progression::StatCurves real_curves() {
    auto rules = wind::content::load_progression_rules(WIND_TEST_CONTENT_DIR);
    EXPECT_TRUE(rules.ok());
    return rules.ok() ? rules.value->curves : wind::progression::StatCurves{};
}

}  // namespace

TEST(DerivedStats, StartingStatsMatchTodaysNumbers) {
    // 모두 10이면 12단계까지의 고정값과 같아야 해요 (HP 100, MP 50, 스태미나 100, 보너스 0)
    const auto d = wind::progression::derive_stats(Stats{}, real_curves());
    EXPECT_EQ(d.max_hp, 100);
    EXPECT_EQ(d.max_mp, 50);
    EXPECT_EQ(d.max_stamina, 100);
    EXPECT_FLOAT_EQ(d.carry_weight, 8.0f);
    EXPECT_FLOAT_EQ(d.strength_bonus, 0.0f);
    EXPECT_FLOAT_EQ(d.intelligence_bonus, 0.0f);
    EXPECT_EQ(d.spell_slots, 2);
}

TEST(DerivedStats, EachStatFeedsItsOwnValues) {
    const auto curves = real_curves();
    Stats stats;
    stats[StatId::Vigor] = 12;  // 명세 8.3 예: 최대 HP 100 → 118
    const auto d = wind::progression::derive_stats(stats, curves);
    EXPECT_EQ(d.max_hp, 118);
    EXPECT_EQ(d.max_mp, 50);  // 다른 값은 그대로
    stats = Stats{};
    stats[StatId::Strength] = 20;
    const auto s = wind::progression::derive_stats(stats, curves);
    EXPECT_FLOAT_EQ(s.strength_bonus, 0.5f);
    EXPECT_FLOAT_EQ(s.carry_weight, 14.0f);
    EXPECT_EQ(s.max_hp, 100);
}

TEST(DerivedStats, BoundsOneAndSixty) {
    const auto curves = real_curves();
    Stats low;
    low.values.fill(1);  // 곡선 첫 점(10)보다 낮으면 첫 값
    const auto d1 = wind::progression::derive_stats(low, curves);
    EXPECT_EQ(d1.max_hp, 100);
    EXPECT_EQ(d1.spell_slots, 2);
    Stats high;
    high.values.fill(60);
    const auto d60 = wind::progression::derive_stats(high, curves);
    EXPECT_EQ(d60.max_hp, 450);
    EXPECT_EQ(d60.max_mp, 210);
    EXPECT_EQ(d60.max_stamina, 200);
    EXPECT_FLOAT_EQ(d60.carry_weight, 30.0f);
    EXPECT_FLOAT_EQ(d60.strength_bonus, 1.2f);
    EXPECT_FLOAT_EQ(d60.intelligence_bonus, 1.2f);
    EXPECT_EQ(d60.spell_slots, 4);
}

TEST(DerivedStats, SpellSlotsRoundDown) {
    const auto curves = real_curves();
    Stats stats;
    stats[StatId::Intelligence] = 19;  // 곡선 2.9 → 2칸
    EXPECT_EQ(wind::progression::derive_stats(stats, curves).spell_slots, 2);
    stats[StatId::Intelligence] = 20;  // 3칸
    EXPECT_EQ(wind::progression::derive_stats(stats, curves).spell_slots, 3);
}

TEST(DerivedStats, ValuesBetweenPoints) {
    const auto curves = real_curves();
    Stats stats;
    stats[StatId::Endurance] = 11;  // 스태미나 103
    stats[StatId::Mind] = 33;       // 150 + 30/10·3 = 159
    const auto d = wind::progression::derive_stats(stats, curves);
    EXPECT_EQ(d.max_stamina, 103);
    EXPECT_EQ(d.max_mp, 159);
}
