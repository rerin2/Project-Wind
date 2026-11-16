// 이 파일은 무엇을 하나요?
// 용수철 흔들림 넉백을 한 박자씩 계산하는 곳이에요.

#include "game/combat/knock_sway.hpp"

namespace wind::game {
namespace {

constexpr float kRestDistance = 0.05f;  // 이보다 가까우면 "제자리"로 봐요 (픽셀)
constexpr float kRestSpeed = 0.5f;      // 이보다 느리면 "멈춤"으로 봐요 (픽셀/초)

}  // namespace

KnockSway::KnockSway(KnockSwayTuning tuning) : tuning_(tuning) {}

void KnockSway::push(engine::Vec2 direction, float knockback) {
    if (knockback <= 0.0f) {
        return;
    }
    const float len = engine::length(direction);
    if (len <= 0.0f) {
        return;  // 방향이 없으면 밀 수 없어요
    }
    // 이미 흔들리는 중이어도 새로 맞은 쪽으로 속도를 바꿔요 (쌓이지 않게)
    velocity_ = direction * (knockback * tuning_.knockback_scale / len);
}

void KnockSway::step(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;  // 히트스톱 중에는 그 자리에 멈춰 있어요
    }
    // 용수철 힘 = -당기는 힘 × 거리 - 마찰 × 속도. 속도를 먼저 바꾸고 위치를 옮겨요(안정적인 계산법)
    const engine::Vec2 accel = offset_ * -tuning_.stiffness + velocity_ * -tuning_.damping;
    velocity_ = velocity_ + accel * dt_seconds;
    offset_ = offset_ + velocity_ * dt_seconds;
    // 너무 멀리 가면 끝에 묶어 두고, 바깥으로 가는 속도는 없애요
    const float distance = engine::length(offset_);
    if (distance > tuning_.max_offset) {
        offset_ = offset_ * (tuning_.max_offset / distance);
        velocity_ = {};
    }
    if (resting()) {  // 거의 멈췄으면 딱 제자리 (오래 미세하게 떨지 않게)
        offset_ = {};
        velocity_ = {};
    }
}

bool KnockSway::resting() const {
    return engine::length(offset_) < kRestDistance && engine::length(velocity_) < kRestSpeed;
}

}  // namespace wind::game
