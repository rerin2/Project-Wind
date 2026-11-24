// 이 파일은 무엇을 하나요?
// 마술 시전 시간(SpellCaster)을 숙제 검사(테스트)해요.
// - 별빛 조각은 0.25초 = 15박자째에 나가요 (한 번만)
// - 마술이 없거나, 동작 데이터가 없거나, 시전 중이면 시작 못 해요
// - 취소하면 안 나가요
// - 광선(15d): 다 모으면 유지 단계, 떼면 끝, 모으는 중에 떼면 안 나감
#include "game/combat/spell_caster.hpp"

#include <gtest/gtest.h>

namespace {

using wind::combat::Spell;
using wind::combat::SpellMotion;
using wind::game::SpellCaster;

constexpr float kDt = 1.0f / 60.0f;

Spell star_shard() {
    Spell spell;
    spell.id = "star_shard";
    spell.mp_cost = 6;
    spell.cast_seconds = 0.25f;
    spell.motion = SpellMotion::Projectile;
    spell.projectile = wind::combat::ProjectileData{360.0f, 240.0f, 6.0f, 6.0f};
    return spell;
}

TEST(SpellCaster, ReleasesOnTheFifteenthStep) {
    SpellCaster caster(star_shard());
    ASSERT_TRUE(caster.start());
    EXPECT_TRUE(caster.busy());
    EXPECT_FALSE(caster.can_start());  // 모으는 중에는 또 시작 못 해요
    for (int i = 1; i < 15; ++i) {
        EXPECT_FALSE(caster.step(kDt)) << i;
    }
    EXPECT_GT(caster.progress(), 0.9f);
    EXPECT_TRUE(caster.step(kDt));   // 15박자 = 0.25초
    EXPECT_FALSE(caster.busy());
    EXPECT_FALSE(caster.step(kDt));  // 한 번만 나가요
    EXPECT_FLOAT_EQ(caster.progress(), 0.0f);
}

TEST(SpellCaster, ZeroCastTimeReleasesOnFirstStep) {
    auto spell = star_shard();
    spell.cast_seconds = 0.0f;
    SpellCaster caster(spell);
    ASSERT_TRUE(caster.start());
    EXPECT_TRUE(caster.step(kDt));
}

// 혜성 광선처럼: 0.5초 모으기, 초당 12 MP, 한 번 맞히기 0.1초
Spell beam_spell() {
    Spell spell;
    spell.id = "comet_beam";
    spell.mp_per_second = 12.0f;
    spell.cast_seconds = 0.5f;
    spell.motion = SpellMotion::Beam;
    spell.beam = wind::combat::BeamData{220.0f, 14.0f, 0.1f};
    return spell;
}

TEST(SpellCaster, NeedsASpellWithMotionData) {
    SpellCaster none;
    EXPECT_FALSE(none.can_start());
    EXPECT_FALSE(none.start());
    EXPECT_FLOAT_EQ(none.mp_cost(), 0.0f);

    auto broken = beam_spell();
    broken.beam.reset();  // 광선인데 광선 모양이 없으면 못 써요
    SpellCaster broken_caster(broken);
    EXPECT_FALSE(broken_caster.can_start());
}

// 광선: 다 모으면 "나갔다"가 아니라 광선 유지로 넘어가고, 누르고 있는 동안 계속돼요 (15d)
TEST(SpellCaster, BeamChargesThenHoldsWhileHeld) {
    SpellCaster caster(beam_spell());
    EXPECT_FLOAT_EQ(caster.mp_cost(), 0.0f);           // 시작할 때 내는 MP는 없어요
    EXPECT_FLOAT_EQ(caster.mp_to_start(), 1.2f);       // 한 번 맞히는 동안의 MP는 있어야 해요
    ASSERT_TRUE(caster.start());
    for (int i = 1; i < 30; ++i) {
        EXPECT_FALSE(caster.step(kDt, true)) << i;
        EXPECT_FALSE(caster.beaming()) << i;
    }
    EXPECT_FALSE(caster.step(kDt, true));  // 30박자 = 0.5초: 광선이 켜져요 (빛 탄처럼 "나감"은 아니에요)
    EXPECT_TRUE(caster.beaming());
    EXPECT_TRUE(caster.busy());
    EXPECT_FLOAT_EQ(caster.progress(), 0.0f);
    for (int i = 0; i < 120; ++i) EXPECT_FALSE(caster.step(kDt, true));
    EXPECT_TRUE(caster.beaming());           // 2초 더 눌러도 계속
    EXPECT_FALSE(caster.step(kDt, false));   // 떼면 끝
    EXPECT_FALSE(caster.busy());
}

// 모으는 중에 떼면 광선은 안 나가요 (개발자와 정함)
TEST(SpellCaster, ReleasingWhileChargingCancelsTheBeam) {
    SpellCaster caster(beam_spell());
    ASSERT_TRUE(caster.start());
    for (int i = 0; i < 10; ++i) caster.step(kDt, true);
    EXPECT_FALSE(caster.step(kDt, false));
    EXPECT_FALSE(caster.busy());
    EXPECT_FALSE(caster.beaming());
}

// 빛 탄은 버튼을 떼도 끝까지 모아서 나가요 (held는 광선만 봐요)
TEST(SpellCaster, ProjectileIgnoresRelease) {
    SpellCaster caster(star_shard());
    ASSERT_TRUE(caster.start());
    for (int i = 1; i < 15; ++i) EXPECT_FALSE(caster.step(kDt, false));
    EXPECT_TRUE(caster.step(kDt, false));
}

TEST(SpellCaster, CancelStopsTheSpell) {
    SpellCaster caster(star_shard());
    ASSERT_TRUE(caster.start());
    caster.step(kDt);
    caster.cancel();
    EXPECT_FALSE(caster.busy());
    for (int i = 0; i < 30; ++i) {
        EXPECT_FALSE(caster.step(kDt));
    }
}

TEST(SpellCaster, SetSpellCancelsAndReportsCost) {
    SpellCaster caster(star_shard());
    EXPECT_FLOAT_EQ(caster.mp_cost(), 6.0f);
    ASSERT_TRUE(caster.start());
    caster.set_spell(std::nullopt);
    EXPECT_FALSE(caster.busy());
    EXPECT_EQ(caster.spell(), nullptr);
}

}  // namespace
