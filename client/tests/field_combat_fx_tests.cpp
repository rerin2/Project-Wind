// 이 파일은 무엇을 하나요?
// 싸움 연출 담당(FieldCombatFx)을 숙제 검사(테스트)해요. (11d)
// - 쓰러진 슬라임이 다 사라지는 박자에, 그 자리에서 사라짐 파티클(fx/slime_burst.json, 16개)이 터져요
// - 사라짐 효과가 없으면 파티클 없이 조용히 사라져요 (게임은 그대로)
#include "game/presentation/field_combat_fx.hpp"

#include "engine/fx/particle_effect_loader.hpp"
#include "wind/content/monster_loader.hpp"

#include <gtest/gtest.h>

#include <map>
#include <string>
#include <vector>

namespace {

using wind::game::FieldCombat;
using wind::game::FieldCombatFx;
using wind::game::PlayerMotion;

constexpr double kDt = 1.0 / 60.0;

wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 20;
    map.height = 20;
    map.walkable.assign(400, 1);
    return map;
}

// 슬라임 하나를 쓰러뜨리고, 다 사라질 때까지 박자마다 연출에 넘겨요. 공중 효과 통의 파티클 수를 돌려줘요.
std::size_t particles_after_vanish(const std::map<std::string, wind::engine::ParticleEffect>* monster_fx) {
    auto data = wind::content::load_monster_file(WIND_TEST_CONTENT_DIR, "monsters/slime_green.json");
    EXPECT_TRUE(data.ok());
    wind::game::Monster slime("slime", std::move(*data.value), {300.0f, 300.0f});
    (void)slime.take_damage(999);  // 처음부터 쓰러진 채
    FieldCombat combat({}, 1, 0.0f, {slime});
    PlayerMotion player({100.0f, 300.0f}, wind::world::Direction::Right, wind::game::MovementTuning{},
                        wind::game::StaminaTuning{}, 0.15);
    wind::game::CombatFxSetup setup;
    setup.monsters = {{2}};
    setup.monster_fx = monster_fx;
    FieldCombatFx fx(std::move(setup));
    wind::engine::ParticleRandom random(7);
    wind::engine::ParticlePool air(256);
    const auto map = open_map();
    bool vanished = false;
    for (int i = 0; i < 60 && !vanished; ++i) {  // 쓰러지는 연출 0.5초
        combat.step(kDt, player, map);
        fx.collect(combat);
        vanished = !combat.vanished().empty();
    }
    EXPECT_TRUE(vanished);
    fx.update(static_cast<float>(kDt), 1.0f, combat, player, player.mover().feet(), random, air);
    return air.size();
}

TEST(FieldCombatFx, VanishingSlimeBurstsIntoParticles) {
    auto burst = wind::engine::load_particle_effect_file(WIND_TEST_CONTENT_DIR, "fx/slime_burst.json");
    ASSERT_TRUE(burst.ok());
    const std::map<std::string, wind::engine::ParticleEffect> effects{{"fx/slime_burst.json", *burst.value}};
    EXPECT_EQ(particles_after_vanish(&effects), 16u);  // 효과 파일의 burst 16개
}

TEST(FieldCombatFx, MissingVanishEffectIsQuiet) {
    EXPECT_EQ(particles_after_vanish(nullptr), 0u);  // 효과가 없어도 문제없이 사라져요
}

}  // namespace
