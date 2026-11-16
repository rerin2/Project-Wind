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
    : shot_(shot), ground_(shot.ground), previous_ground_(shot.ground) {
    const float len = engine::length(shot_.direction);
    shot_.direction = len > 0.0f ? shot_.direction * (1.0f / len) : engine::Vec2{1.0f, 0.0f};
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
    if (!alive() || dt_seconds <= 0.0f) {
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
