// 이 파일은 무엇을 하나요?
// 경직·무적 시간을 세고, 박자마다 넉백으로 밀려날 거리를 계산하는 곳이에요.

#include "game/entities/hit_reaction.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

HitReaction::HitReaction(HitTuning tuning) : tuning_(tuning) {}

void HitReaction::start(engine::Vec2 direction, float knockback) {
    stagger_left_ = std::max(0.0f, tuning_.stagger_seconds);           // 경직 처음부터
    invulnerable_left_ = std::max(0.0f, tuning_.invulnerable_seconds); // 무적 처음부터
    push_direction_ = engine::normalized(direction);                    // 길이 1 (0이면 안 밀려요)
    push_speed_ = std::isfinite(knockback) ? std::max(0.0f, knockback) : 0.0f;
}

engine::Vec2 HitReaction::step(float dt_seconds) {
    const float dt = std::max(0.0f, dt_seconds);
    invulnerable_left_ = std::max(0.0f, invulnerable_left_ - dt);  // 무적 시간 줄이기
    if (stagger_left_ <= 0.0f) return {};                           // 경직이 끝났으면 밀리지 않아요
    const float total = tuning_.stagger_seconds;                    // 경직 전체 시간
    // 빠르기는 남은 시간에 비례해요: v(남은 t) = v0 × t / 전체. 이 박자 동안 간 거리 = 그 넓이(사다리꼴)
    const float before = stagger_left_;
    const float after = std::max(0.0f, stagger_left_ - dt);
    stagger_left_ = after;
    if (total <= 0.0f) return {};
    const float distance = push_speed_ * (before * before - after * after) / (2.0f * total);
    return push_direction_ * distance;
}

}  // namespace wind::game
