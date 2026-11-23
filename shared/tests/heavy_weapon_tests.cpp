// 이 파일은 무엇을 하나요?
// 무거움 상태(명세 8.4, 15a): 힘이 모자라면 대검이 무거워지고, 공격 시간 ×1.3, 스태미나 ×1.5가 되는지 검사해요.
#include "wind/combat/heavy_weapon.hpp"

#include "wind/content/progression_loader.hpp"
#include "wind/content/weapon_loader.hpp"
#include "wind/progression/derived_stats.hpp"

#include <gtest/gtest.h>

namespace {

using wind::progression::StatId;
using wind::progression::Stats;

wind::progression::StatCurves real_curves() {
    auto rules = wind::content::load_progression_rules(WIND_TEST_CONTENT_DIR);
    EXPECT_TRUE(rules.ok());
    return rules.ok() ? rules.value->curves : wind::progression::StatCurves{};
}

wind::combat::Weapon real_weapon(const char* path) {
    auto loaded = wind::content::load_weapon_file(WIND_TEST_CONTENT_DIR, path);
    EXPECT_TRUE(loaded.ok()) << path;
    return loaded.ok() ? *loaded.value : wind::combat::Weapon{};
}

}  // namespace

// 완료 기준: 힘이 모자란 대검은 무거움 (시작 힘 10 → 들 수 있는 무게 8 < 14)
TEST(HeavyWeapon, GreatswordIsHeavyAtStartingStrength) {
    const auto curves = real_curves();
    const auto great = real_weapon("weapons/greatsword.json");
    const auto start = wind::progression::derive_stats(Stats{}, curves, great.weight);
    EXPECT_FLOAT_EQ(start.equipped_weight, 14.0f);
    EXPECT_TRUE(start.heavy);
    // 롱소드(6)와 단검(2)은 시작 힘으로도 가벼워요
    EXPECT_FALSE(wind::progression::derive_stats(Stats{}, curves, 6.0f).heavy);
    EXPECT_FALSE(wind::progression::derive_stats(Stats{}, curves, 2.0f).heavy);
}

// 힘 20(들 수 있는 무게 14)부터 대검을 제대로 써요 (같으면 무겁지 않아요)
TEST(HeavyWeapon, StrengthTwentyCarriesTheGreatsword) {
    const auto curves = real_curves();
    Stats strong;
    strong[StatId::Strength] = 20;
    const auto d = wind::progression::derive_stats(strong, curves, 14.0f);
    EXPECT_FLOAT_EQ(d.carry_weight, 14.0f);
    EXPECT_FALSE(d.heavy);
    strong[StatId::Strength] = 19;
    EXPECT_TRUE(wind::progression::derive_stats(strong, curves, 14.0f).heavy);
}

TEST(HeavyWeapon, BadWeightCountsAsNothing) {
    const auto d = wind::progression::derive_stats(Stats{}, real_curves(), -5.0f);
    EXPECT_FLOAT_EQ(d.equipped_weight, 0.0f);
    EXPECT_FALSE(d.heavy);
}

TEST(HeavyWeapon, HeavyStretchesTimesAndStaminaOnly) {
    const auto great = real_weapon("weapons/greatsword.json");
    ASSERT_EQ(great.combo.size(), 2u);
    const auto heavy = wind::combat::wield(great, true);
    EXPECT_FLOAT_EQ(heavy.stamina_per_attack, 26.0f * 1.5f);
    const auto& a = great.combo[0];
    const auto& h = heavy.combo[0];
    EXPECT_FLOAT_EQ(h.windup, a.windup * 1.3f);
    EXPECT_FLOAT_EQ(h.active, a.active * 1.3f);
    EXPECT_FLOAT_EQ(h.recovery, a.recovery * 1.3f);
    EXPECT_FLOAT_EQ(h.cancel_start, a.cancel_start * 1.3f);
    EXPECT_FLOAT_EQ(h.cancel_end, a.cancel_end * 1.3f);
    // 공격력·위력·넉백은 그대로
    EXPECT_FLOAT_EQ(heavy.base_power, great.base_power);
    EXPECT_FLOAT_EQ(h.power, a.power);
    EXPECT_FLOAT_EQ(h.knockback, a.knockback);
}

TEST(HeavyWeapon, LightWeaponIsUnchanged) {
    const auto sword = real_weapon("weapons/longsword.json");
    const auto held = wind::combat::wield(sword, false);
    EXPECT_FLOAT_EQ(held.stamina_per_attack, sword.stamina_per_attack);
    EXPECT_FLOAT_EQ(held.combo[0].windup, sword.combo[0].windup);
}
