// 이 파일은 무엇을 하나요?
// 몬스터 한 마리의 대기·배회를 박자마다 계산하는 곳이에요. (11a)
// 값의 이유는 docs/GAMEPLAY.md "Monsters" 절에 있어요.

#include "game/monsters/monster.hpp"

#include "game/combat/combat_random.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {
namespace {

// 걸어갈 점을 고를 때 몇 번까지 다시 굴려 볼지. 둥근 범위 밖이거나 벽 속 점이면 다시 굴려요.
constexpr int kTargetTries = 8;
// 남은 거리가 이보다 짧은 점은 "이미 도착"이라 걷지 않고 다시 쉬어요 (픽셀)
constexpr float kArriveDistance = 1.0f;
// 한 박자에 가야 할 거리의 절반도 못 가면 "막힘"으로 세요
constexpr float kStuckRatio = 0.5f;
// 막힌 채로 이만큼 지나면 포기하고 쉬어요 (초). 벽을 따라 미끄러질 시간은 줘요.
constexpr float kGiveUpSeconds = 0.5f;
// 배회 중 벽 모서리 보정 (픽셀). 주인공(MovementTuning)보다 작게: 몬스터는 길을 정확히 몰라도 돼요.
constexpr float kCornerAssist = 2.0f;

}  // namespace

Monster::Monster(std::string name, combat::MonsterData data, engine::Vec2 home)
    : name_(std::move(name)),
      data_(std::move(data)),
      home_(home),
      feet_(home),
      previous_feet_(home),
      target_(home),
      idle_left_(data_.wander.pause_min) {}  // 처음 쉬는 시간은 최소값 (주사위는 박자 안에서만 굴려요)

void Monster::step(float dt_seconds, const world::MapData& map, CombatRandom& random) {
    previous_feet_ = feet_;                // 부드럽게 그리기: 이번 박자 시작 위치
    velocity_ = {};                        // 걷지 않으면 빠르기 0
    if (!(dt_seconds > 0.0f)) return;      // 히트스톱(0)이나 이상한 값이면 그대로
    switch (state_) {
        case MonsterState::Idle:
            idle_left_ -= dt_seconds;              // 쉬는 시간 줄이기
            if (idle_left_ <= 0.0f) {
                start_wander(map, random);         // 다 쉬었으면 걸어갈 점 고르기
            }
            break;
        case MonsterState::Wander:
            walk(dt_seconds, map, random);         // 고른 점으로 걷기
            break;
    }
}

engine::Vec2 Monster::interpolated_feet(float alpha) const {
    return engine::lerp(previous_feet_, feet_, std::clamp(alpha, 0.0f, 1.0f));
}

engine::RectF Monster::hurtbox() const {
    return {feet_.x + data_.hurtbox.x, feet_.y + data_.hurtbox.y, data_.hurtbox.w, data_.hurtbox.h};
}

void Monster::start_idle(CombatRandom& random) {
    state_ = MonsterState::Idle;
    const auto& wander = data_.wander;
    // 쉬는 시간 = 최소 + (최대 - 최소) × 주사위(0~1)
    idle_left_ = wander.pause_min + (wander.pause_max - wander.pause_min) * random.next_unit();
}

void Monster::start_wander(const world::MapData& map, CombatRandom& random) {
    const float radius = data_.wander.radius;
    for (int i = 0; i < kTargetTries; ++i) {
        // 네모 안의 점을 굴리고, 둥근 범위 밖이면 버려요. (sin·cos 없이 어느 컴퓨터에서나 같은 값)
        const engine::Vec2 offset{(random.next_unit() * 2.0f - 1.0f) * radius,
                                  (random.next_unit() * 2.0f - 1.0f) * radius};
        if (offset.x * offset.x + offset.y * offset.y > radius * radius) continue;  // 동그라미 밖
        const engine::Vec2 target = home_ + offset;                                // 제자리 기준 점
        if (box_blocked(map, body_box(target, body()))) continue;                  // 벽 속이면 다시
        if (engine::length(target - feet_) < kArriveDistance) continue;            // 이미 거기 있음
        target_ = target;                       // 좋은 점을 골랐어요
        state_ = MonsterState::Wander;
        stuck_seconds_ = 0.0f;
        return;
    }
    start_idle(random);  // 좋은 점이 없으면 조금 더 쉬어요
}

void Monster::walk(float dt_seconds, const world::MapData& map, CombatRandom& random) {
    const engine::Vec2 to_target = target_ - feet_;                      // 남은 길
    const float remaining = engine::length(to_target);                    // 남은 거리
    const float stride = data_.wander.speed * dt_seconds;                 // 이번 박자에 갈 거리
    const bool last_step = remaining <= stride;                           // 이번에 도착하나
    const engine::Vec2 delta = last_step ? to_target : engine::normalized(to_target) * stride;
    const MoveResult moved = move_and_slide(map, body(), feet_, delta, kCornerAssist);  // 벽 미끄러짐
    const float travelled = engine::length(moved.feet - feet_);           // 실제로 간 거리
    velocity_ = (moved.feet - feet_) * (1.0f / dt_seconds);
    feet_ = moved.feet;
    distance_moved_ += travelled;
    if (last_step && !moved.hit_x && !moved.hit_y) {  // 막힘 없이 도착
        start_idle(random);
        return;
    }
    // 가야 할 거리의 절반도 못 갔으면 막힌 시간을 세고, 오래 막히면 포기해요
    const float expected = std::min(stride, remaining);
    stuck_seconds_ = travelled < expected * kStuckRatio ? stuck_seconds_ + dt_seconds : 0.0f;
    if (stuck_seconds_ >= kGiveUpSeconds) {
        start_idle(random);
    }
}

}  // namespace wind::game
