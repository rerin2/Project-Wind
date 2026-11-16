// 이 파일은 무엇을 하나요?
// 날아가는 탄 규칙(명세 9.2, 9단계 완료 조건)을 숙제 검사(테스트)해요.
// - 정해진 빠르기로 날고, range만큼 날면 사라져요
// - 벽(지형)이나 맵 밖에 닿으면 사라져요
// - 대상 하나에 맞으면 사라지고, 그 뒤 대상은 안 맞아요
// - 빨라도 얇은 벽을 건너뛰지 않아요
// - 허수아비가 막은 칸은 탄에게 벽이 아니에요
#include "game/combat/projectile.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::RectF;
using wind::engine::Vec2;
using wind::game::Projectile;
using wind::game::ProjectileEnd;
using wind::game::ProjectileShot;

constexpr float kDt = 1.0f / 60.0f;
constexpr int kTile = 32;

// 20x5칸 열린 맵 (벽 없음)
wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = kTile;
    map.width = 20;
    map.height = 5;
    map.walkable.assign(static_cast<std::size_t>(20 * 5), 1);
    return map;
}

void set_wall(wind::world::MapData& map, int x, int y) {
    map.walkable[static_cast<std::size_t>(y * map.width + x)] = 0;
}

// 별빛 조각과 같은 탄: 360 px/s, 240 px, 6x6, 2칸 가운데에서 오른쪽으로
ProjectileShot star_shard(Vec2 ground = {48.0f, 80.0f}) {
    ProjectileShot shot;
    shot.ground = ground;
    shot.direction = {1.0f, 0.0f};
    shot.lift = 14.0f;
    shot.data.speed = 360.0f;
    shot.data.range = 240.0f;
    shot.data.width = 6.0f;
    shot.data.height = 6.0f;
    return shot;
}

const Projectile::HitTest kNoTargets = [](const RectF&) { return false; };

TEST(Projectile, FliesAtSpeedAndEndsAtRange) {
    const auto map = open_map();
    Projectile shard(star_shard());
    shard.step(kDt, map, kNoTargets);
    EXPECT_NEAR(shard.ground().x, 48.0f + 6.0f, 1e-3f);  // 360/60 = 한 박자에 6픽셀
    EXPECT_NEAR(shard.previous_ground().x, 48.0f, 1e-3f);
    EXPECT_TRUE(shard.alive());
    for (int i = 0; i < 38; ++i) {  // 39박자 = 234픽셀: 아직 날아요
        shard.step(kDt, map, kNoTargets);
    }
    EXPECT_TRUE(shard.alive());
    shard.step(kDt, map, kNoTargets);  // 40박자 = 240픽셀: 끝
    EXPECT_FALSE(shard.alive());
    EXPECT_EQ(shard.end(), ProjectileEnd::Range);
    EXPECT_NEAR(shard.travelled(), 240.0f, 1e-2f);  // 거리를 넘어가지 않아요
}

// 오른쪽 벽 칸(5,2)에 닿는 걸음에서 사라져요. 벽 안으로 한 걸음(3픽셀) 넘게 들어가지 않아요.
TEST(Projectile, StopsAtWall) {
    auto map = open_map();
    set_wall(map, 5, 2);
    Projectile shard(star_shard({48.0f, 80.0f}));  // 1칸, 2줄
    for (int i = 0; i < 60 && shard.alive(); ++i) {
        shard.step(kDt, map, kNoTargets);
    }
    EXPECT_EQ(shard.end(), ProjectileEnd::Wall);
    EXPECT_GE(shard.ground().x, 160.0f);         // 벽 칸 시작
    EXPECT_LT(shard.ground().x, 160.0f + 3.1f);  // 걸음 하나 안
}

TEST(Projectile, StopsAtMapEdge) {
    const auto map = open_map();
    auto shot = star_shard({600.0f, 80.0f});  // 맵 오른쪽 끝(640) 가까이
    Projectile shard(shot);
    for (int i = 0; i < 60 && shard.alive(); ++i) {
        shard.step(kDt, map, kNoTargets);
    }
    EXPECT_EQ(shard.end(), ProjectileEnd::Wall);
}

// 아주 빠른 탄(한 박자 100픽셀)도 1칸 벽을 건너뛰지 않아요.
TEST(Projectile, FastShotDoesNotTunnel) {
    auto map = open_map();
    set_wall(map, 4, 2);
    auto shot = star_shard({48.0f, 80.0f});
    shot.data.speed = 6000.0f;  // 한 박자에 100픽셀
    shot.data.range = 600.0f;
    Projectile shard(shot);
    shard.step(kDt, map, kNoTargets);
    shard.step(kDt, map, kNoTargets);
    EXPECT_EQ(shard.end(), ProjectileEnd::Wall);
    EXPECT_LT(shard.ground().x, 128.0f + 3.1f);  // 벽 칸(128~160) 들머리에서 멈춤
}

// 첫 대상에 맞으면 사라지고, 같은 박자 안이어도 다음 대상은 묻지 않아요.
TEST(Projectile, FirstTargetEndsIt) {
    const auto map = open_map();
    int asked_after_hit = 0;
    bool hit = false;
    const Projectile::HitTest targets = [&](const RectF& r) {
        if (hit) ++asked_after_hit;
        // 대상 둘: x 70~80, x 80~90 (몸 높이 y 60~70)
        const bool overlap = r.x + r.w > 70.0f && r.x < 90.0f && r.y + r.h > 60.0f && r.y < 70.0f;
        if (overlap) hit = true;
        return overlap;
    };
    Projectile shard(star_shard());
    for (int i = 0; i < 10 && shard.alive(); ++i) {
        shard.step(kDt, map, targets);
    }
    EXPECT_EQ(shard.end(), ProjectileEnd::Target);
    EXPECT_EQ(asked_after_hit, 0);
    EXPECT_LT(shard.rect().x, 70.0f + 1.0f);  // 첫 대상 들머리에서 멈춤
}

// 몸 높이 네모: ground에서 lift만큼 위, 크기 6x6
TEST(Projectile, RectIsAtBodyHeight) {
    const Projectile shard(star_shard({100.0f, 80.0f}));
    const RectF r = shard.rect();
    EXPECT_FLOAT_EQ(r.x, 97.0f);
    EXPECT_FLOAT_EQ(r.y, 80.0f - 14.0f - 3.0f);
    EXPECT_FLOAT_EQ(r.w, 6.0f);
    EXPECT_FLOAT_EQ(shard.center().y, 66.0f);
}

// 허수아비 칸은 사람에게는 막힌 칸이지만 탄에게는 열린 칸 (그래야 허수아비에 맞아요)
TEST(Projectile, EntityTileIsNotAWall) {
    auto map = open_map();
    set_wall(map, 4, 2);  // 로더처럼 허수아비 칸을 막아요
    map.entities.push_back({wind::world::EntityType::TrainingDummy, "d", {4, 2}, "props/d.json"});
    EXPECT_FALSE(wind::game::projectile_blocked(map, {140.0f, 80.0f}));
    set_wall(map, 6, 2);  // 진짜 벽
    EXPECT_TRUE(wind::game::projectile_blocked(map, {200.0f, 80.0f}));
    EXPECT_TRUE(wind::game::projectile_blocked(map, {-1.0f, 80.0f}));  // 맵 밖
}

// 방향 길이가 1이 아니어도 맞춰 주고, (0,0)이면 오른쪽으로 날아요.
TEST(Projectile, DirectionIsNormalized) {
    const auto map = open_map();
    auto shot = star_shard();
    shot.direction = {0.0f, 5.0f};
    Projectile down(shot);
    down.step(kDt, map, kNoTargets);
    EXPECT_NEAR(down.ground().y, 86.0f, 1e-3f);
    shot.direction = {};
    Projectile still(shot);
    still.step(kDt, map, kNoTargets);
    EXPECT_NEAR(still.ground().x, 54.0f, 1e-3f);
}

}  // namespace
