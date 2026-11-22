// 이 파일은 무엇을 하나요?
// 파생값 → 주인공 몸 숫자 묶음 검사 (13a): 최대값만 바뀌고 나머지는 기본값 그대로인가.
#include "game/progression/hero_tuning.hpp"

#include <gtest/gtest.h>

TEST(HeroTuning, CopiesOnlyMaximums) {
    wind::progression::DerivedStats derived;
    derived.max_hp = 118;
    derived.max_mp = 65;
    derived.max_stamina = 106;
    const auto health = wind::game::health_tuning(derived);
    const auto mana = wind::game::mana_tuning(derived);
    const auto stamina = wind::game::stamina_tuning(derived);
    EXPECT_EQ(health.max, 118);
    EXPECT_EQ(health.floor, wind::game::HealthTuning{}.floor);
    EXPECT_FLOAT_EQ(mana.max, 65.0f);
    EXPECT_FLOAT_EQ(mana.regen_per_second, wind::game::ManaTuning{}.regen_per_second);
    EXPECT_FLOAT_EQ(stamina.max, 106.0f);
    EXPECT_FLOAT_EQ(stamina.dodge_cost, wind::game::StaminaTuning{}.dodge_cost);
}
