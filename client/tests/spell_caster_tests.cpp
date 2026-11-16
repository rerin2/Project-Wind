// 이 파일은 무엇을 하나요?
// 마술 시전 시간(SpellCaster)을 숙제 검사(테스트)해요.
// - 별빛 조각은 0.25초 = 15박자째에 나가요 (한 번만)
// - 마술이 없거나, 빛 탄이 아닌 마술이거나, 시전 중이면 시작 못 해요
// - 취소하면 안 나가요
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

TEST(SpellCaster, NeedsAProjectileSpell) {
    SpellCaster none;
    EXPECT_FALSE(none.can_start());
    EXPECT_FALSE(none.start());
    EXPECT_FLOAT_EQ(none.mp_cost(), 0.0f);

    auto beam = star_shard();
    beam.motion = SpellMotion::Beam;
    beam.projectile.reset();
    beam.beam = wind::combat::BeamData{160.0f, 12.0f, 0.1f};
    SpellCaster beam_caster(beam);
    EXPECT_FALSE(beam_caster.can_start());  // 광선은 15단계
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
