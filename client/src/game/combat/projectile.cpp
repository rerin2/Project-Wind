// 이 파일은 무엇을 하나요?
// 탄 하나를 잘게 나눈 걸음으로 옮기며 "대상 -> 벽 -> 거리" 순서로 검사하는 곳이에요.

#include "game/combat/projectile.hpp"

#include "engine/map/coordinates.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kMinStepPixels = 1.0f;  // 걸음이 너무 작아지지 않게 (아주 작은 탄도 1픽셀씩)
constexpr float kRangeEpsilon = 1e-3f;  // 거리 끝 판정 여유 (픽셀)
// 별무리 돌기·휘기 (15c). 이유는 docs/GAMEPLAY.md "Star swarm" 절에 있어요.
constexpr float kTwoPi = 6.28318531f;
constexpr float kOrbitRadius = 22.0f;         // 발에서 별까지 (몸 상자 반쯤 밖, 픽셀)
constexpr float kOrbitFlatten = 0.6f;         // 세로 줄이기 (비스듬히 본 원)
constexpr float kOrbitTurnsPerSecond = 0.9f;  // 1초에 0.9바퀴 (1.5초면 한 바퀴 남짓)
constexpr float kMaxTurnPerSecond = 8.0f;     // homing 1일 때 1초에 휠 수 있는 각도 (라디안, 약 460°)

// 이 칸을 막은 게 물건(허수아비)뿐인가요? 그러면 지형은 열린 칸이에요.
bool blocked_only_by_entity(const world::MapData& map, world::TileCoord tile) {
    return std::ranges::any_of(map.entities, [tile](const world::MapEntity& entity) {
        return world::blocks_movement(entity.type) && entity.tile == tile;
    });
}

}  // namespace

bool projectile_blocked(const world::MapData& map, engine::Vec2 ground) {
    const auto tile = engine::world_to_tile(ground, map.tile_size);  // 점이 있는 칸
    if (!map.in_bounds(tile)) {
        return true;  // 맵 밖은 벽
    }
    if (map.is_walkable(tile)) {
        return false;  // 열린 칸
    }
    return !blocked_only_by_entity(map, tile);  // 지형 벽이면 막힘
}

// 방향은 길이 1로 맞추고, 방향이 없으면(0,0) 오른쪽으로 날려요.
Projectile::Projectile(ProjectileShot shot)
    : shot_(shot), ground_(shot.ground), previous_ground_(shot.ground),
      orbit_left_(std::max(0.0f, shot.orbit_seconds)), orbit_angle_(shot.orbit_angle) {
    const float len = engine::length(shot_.direction);
    shot_.direction = len > 0.0f ? shot_.direction * (1.0f / len) : engine::Vec2{1.0f, 0.0f};
}

void Projectile::orbit(engine::Vec2 hub, float dt_seconds, const HitTest& hit_test) {
    previous_ground_ = ground_;
    if (!orbiting() || dt_seconds <= 0.0f) return;
    orbit_left_ -= dt_seconds;
    orbit_angle_ += kOrbitTurnsPerSecond * kTwoPi * dt_seconds;  // 시계 반대 방향으로 돌아요
    // 땅 위 납작한 원: 위에서 비스듬히 보는 화면이라 세로를 줄여야 둥글게 감싸 보여요
    ground_ = hub + engine::Vec2{std::cos(orbit_angle_) * kOrbitRadius,
                                 std::sin(orbit_angle_) * kOrbitRadius * kOrbitFlatten};
    if (hit_test && hit_test(rect())) end_ = ProjectileEnd::Target;  // 도는 동안 닿아도 맞혀요 (개발자와 정함)
    if (!orbiting() && alive()) shot_.ground = ground_;  // 다 돌았어요: 여기서부터 날아가요
}

void Projectile::aim(engine::Vec2 direction) {
    const float len = engine::length(direction);
    if (len > 0.0f) shot_.direction = direction * (1.0f / len);
}

void Projectile::steer(engine::Vec2 target, float dt_seconds) {
    if (shot_.data.homing <= 0.0f || dt_seconds <= 0.0f) return;
    const engine::Vec2 want = target - center();
    if (engine::length(want) <= 0.0f) return;
    // 지금 방향과 가고 싶은 방향 사이 각도 (-π ~ π)
    const float now = std::atan2(shot_.direction.y, shot_.direction.x);
    float turn = std::atan2(want.y, want.x) - now;
    turn = std::remainder(turn, kTwoPi);  // 가까운 쪽으로 돌아요
    const float limit = shot_.data.homing * kMaxTurnPerSecond * dt_seconds;  // 이번 박자에 돌 수 있는 만큼
    const float angle = now + std::clamp(turn, -limit, limit);
    shot_.direction = {std::cos(angle), std::sin(angle)};
}

engine::Vec2 Projectile::center() const {
    return {ground_.x, ground_.y - shot_.lift};
}

engine::RectF Projectile::rect() const {
    const engine::Vec2 c = center();
    return {c.x - shot_.data.width * 0.5f, c.y - shot_.data.height * 0.5f, shot_.data.width,
            shot_.data.height};
}

void Projectile::step(float dt_seconds, const world::MapData& map, const HitTest& hit_test) {
    previous_ground_ = ground_;  // 그림을 부드럽게 이을 때 쓰는 지난 위치
    if (!alive() || orbiting() || dt_seconds <= 0.0f) {  // 도는 중이면 orbit이 옮겨요 (15c)
        return;
    }
    // 이번 박자에 갈 거리. 남은 거리(range)보다 멀리 가지 않아요.
    float remaining = std::min(shot_.data.speed * dt_seconds, shot_.data.range - travelled_);
    // 한 걸음 = 탄 크기의 반 (탄이 얇은 벽·대상을 건너뛰지 않게)
    const float step = std::max(kMinStepPixels, std::min(shot_.data.width, shot_.data.height) * 0.5f);
    while (remaining > 0.0f && alive()) {
        const float distance = std::min(step, remaining);
        remaining -= distance;
        if (!advance(distance, map, hit_test)) {
            return;
        }
    }
    // 소수를 여러 번 더하면 range보다 아주 조금 모자랄 수 있어서 작은 여유를 둬요.
    if (alive() && travelled_ >= shot_.data.range - kRangeEpsilon) {
        end_ = ProjectileEnd::Range;  // 다 날았어요
    }
}

bool Projectile::advance(float distance, const world::MapData& map, const HitTest& hit_test) {
    ground_ = ground_ + shot_.direction * distance;  // 한 걸음 앞으로
    travelled_ += distance;
    // 1) 대상 먼저: 벽 바로 앞에 선 허수아비도 맞을 수 있게
    if (hit_test && hit_test(rect())) {
        end_ = ProjectileEnd::Target;
        return false;
    }
    // 2) 벽 (지형, 맵 밖)
    if (projectile_blocked(map, ground_)) {
        end_ = ProjectileEnd::Wall;
        return false;
    }
    return true;
}

}  // namespace wind::game
