// 이 파일은 무엇을 하나요?
// 마술 연출(SpellView)의 계산 부분을 숙제 검사(테스트)해요. 그리기는 화면으로 확인해요.
// - 빛 탄은 지난 박자와 지금 박자 사이를 이어서 몸 높이에 그려요
// - 빛 탄마다 빛 하나, 시전 중이면 손끝에 모은 만큼 밝은 빛 하나
// - 빛을 내지 않는 마술이면 빛이 없어요
#include "game/presentation/spell_view.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace {

using wind::engine::Vec2;
using wind::game::CastGlow;
using wind::game::Projectile;
using wind::game::ProjectileShot;
using wind::game::SpellView;

// 오른쪽으로 한 박자 날린 별빛 조각 (48 -> 54, 몸 높이 14)
// light: 탄이 들고 다니는 마술 빛 (15b)
Projectile flown_shard(std::optional<wind::combat::SpellLight> light = std::nullopt) {
    ProjectileShot shot;
    shot.light = light;
    shot.ground = {48.0f, 80.0f};
    shot.lift = 14.0f;
    shot.data.speed = 360.0f;
    shot.data.range = 240.0f;
    shot.data.width = 6.0f;
    shot.data.height = 6.0f;
    Projectile shard(shot);
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 10;
    map.height = 5;
    map.walkable.assign(50, 1);
    shard.step(1.0f / 60.0f, map, [](const wind::engine::RectF&) { return false; });
    return shard;
}

wind::combat::SpellLight ice_light() {
    return {36.0f, {168, 220, 239, 255}, 0.8f};
}

TEST(SpellView, DrawnCenterBlendsSteps) {
    const auto shard = flown_shard();
    EXPECT_FLOAT_EQ(wind::game::drawn_center(shard, 0.0f).x, 48.0f);
    EXPECT_FLOAT_EQ(wind::game::drawn_center(shard, 0.5f).x, 51.0f);
    EXPECT_FLOAT_EQ(wind::game::drawn_center(shard, 1.0f).x, 54.0f);
    EXPECT_FLOAT_EQ(wind::game::drawn_center(shard, 2.0f).x, 54.0f);  // 1보다 크면 잘라요
    EXPECT_FLOAT_EQ(wind::game::drawn_center(shard, 1.0f).y, 66.0f);  // 80 - 14
}

TEST(SpellView, OneLightPerShardPlusCastGlow) {
    SpellView view(nullptr, nullptr, {});
    const std::vector<Projectile> shots{flown_shard(ice_light()), flown_shard(ice_light())};
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(16);
    view.update(0.016f, shots, 1.0f, {}, std::nullopt, random, pool);
    auto lights = view.lights(shots, 1.0f);
    ASSERT_EQ(lights.size(), 2u);
    EXPECT_FLOAT_EQ(lights[0].radius, 36.0f);
    EXPECT_FLOAT_EQ(lights[0].intensity, 0.8f);
    EXPECT_FLOAT_EQ(lights[0].position.x, 54.0f);
    EXPECT_EQ(lights[0].color.g, 220);

    // 시전 중: 반쯤 모았으면 반 크기·반 밝기의 손끝 빛이 더해져요
    view.update(0.016f, {}, 1.0f, {}, CastGlow{{10.0f, 20.0f}, 0.5f, ice_light()}, random, pool);
    lights = view.lights({}, 1.0f);
    ASSERT_EQ(lights.size(), 1u);
    EXPECT_FLOAT_EQ(lights[0].radius, 18.0f);
    EXPECT_FLOAT_EQ(lights[0].intensity, 0.4f);
    EXPECT_FLOAT_EQ(lights[0].position.y, 20.0f);
}

TEST(SpellView, NoLightWhenSpellHasNone) {
    SpellView view(nullptr, nullptr, {});
    const std::vector<Projectile> shots{flown_shard()};
    EXPECT_TRUE(view.lights(shots, 1.0f).empty());
}

// 효과가 있으면 꼬리를 뿜고, 사라진 자리에서 반짝을 터뜨려요.
TEST(SpellView, EmitsTrailAndBurst) {
    wind::engine::ParticleEffect trail;
    trail.lifetime = {1.0f, 1.0f};
    trail.rate = 120.0f;  // 1초에 120개 = 한 화면(1/60초)에 2개 (소수 오차가 있어도 1개 이상)
    wind::engine::ParticleEffect burst;
    burst.lifetime = {1.0f, 1.0f};
    burst.burst = 5;
    SpellView view(&trail, &burst, {});
    wind::engine::ParticleRandom random(1);
    wind::engine::ParticlePool pool(64);
    const std::vector<Projectile> shots{flown_shard()};
    const std::vector<wind::game::ProjectileEndEvent> ends{{{60.0f, 60.0f}, wind::game::ProjectileEnd::Wall}};
    view.update(1.0f / 60.0f, shots, 1.0f, ends, std::nullopt, random, pool);
    EXPECT_GE(pool.size(), 5u + 1u);
}

}  // namespace
