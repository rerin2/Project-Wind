// 이 파일은 무엇을 하나요?
// 캐릭터의 "다리"(Mover)가 박자마다 속도를 바꾸고, 구르고, 벽을 피해 움직이는 방법이에요.

#include "game/entities/mover.hpp"

#include "game/entities/facing.hpp"

#include <algorithm>

namespace wind::game {
namespace {

using engine::Vec2;

// 지금 속도(current)를 목표 속도(target)쪽으로 최대 max_change만큼만 바꿔요.
// 한 번에 확 바뀌지 않아서, 출발과 멈춤이 부드러워요.
Vec2 move_towards(Vec2 current, Vec2 target, float max_change) {
    const Vec2 diff = target - current;
    const float distance = engine::length(diff);
    if (distance <= max_change || distance == 0.0f) {
        return target;
    }
    return current + diff * (max_change / distance);
}

// 이보다 느리면 "서 있다"고 봐요 (픽셀/초).
constexpr float kIdleSpeed = 1.0f;

}  // namespace

Mover::Mover(Vec2 feet, world::Direction facing, MovementTuning tuning)
    : tuning_(tuning), feet_(feet), previous_feet_(feet), facing_(facing) {}

bool Mover::step(double dt_seconds, Vec2 move_input, bool dodge, const world::MapData& map,
                 bool run) {
    const auto dt = static_cast<float>(dt_seconds);
    previous_feet_ = feet_;  // 화면 보간용으로 전 박자 위치를 기억해요
    dodge_cooldown_ = std::max(0.0f, dodge_cooldown_ - dt);

    bool started_dodge = false;
    if (dodge && can_dodge()) {
        start_dodge(move_input);
        started_dodge = true;
    }

    if (state_ == State::Dodging) {
        dodge_elapsed_ += dt;
        velocity_ = dodge_direction_ * tuning_.dodge_speed;
        if (dodge_elapsed_ >= tuning_.dodge_seconds) {
            // 구르기 끝: 걷기 최고 속도로 줄여서 이어 걸어요 (뚝 멈추지 않게)
            state_ = State::Walking;
            velocity_ = dodge_direction_ * tuning_.max_speed;
            dodge_cooldown_ = tuning_.dodge_cooldown_seconds;
        }
    } else {
        update_velocity(dt, move_input, run);
        update_facing(move_input);
    }

    // 실제로 옮기기: 벽에 막힌 축은 속도도 0으로 만들어요 (벽에 계속 밀어붙이지 않게)
    const auto moved =
        move_and_slide(map, tuning_.body, feet_, velocity_ * dt, tuning_.corner_assist);
    feet_ = moved.feet;
    if (moved.hit_x) {
        velocity_.x = 0.0f;
    }
    if (moved.hit_y) {
        velocity_.y = 0.0f;
    }

    if (state_ != State::Dodging) {
        const bool moving = speed() > kIdleSpeed;
        const bool pushing = engine::length(move_input) > 0.0f;
        state_ = !moving ? State::Idle : (run && pushing ? State::Running : State::Walking);
    }
    return started_dodge;
}

bool Mover::can_dodge() const {
    return state_ != State::Dodging && dodge_cooldown_ <= 0.0f;
}

void Mover::teleport(Vec2 feet) {
    feet_ = feet;
    previous_feet_ = feet;
    velocity_ = {};
    state_ = State::Idle;
}

Vec2 Mover::interpolated_feet(float alpha) const {
    return engine::lerp(previous_feet_, feet_, std::clamp(alpha, 0.0f, 1.0f));
}

bool Mover::invulnerable() const {
    return state_ == State::Dodging && dodge_elapsed_ < tuning_.dodge_invulnerable_seconds;
}

void Mover::start_dodge(Vec2 move_input) {
    // 방향키를 누르고 있으면 그쪽으로, 아니면 바라보는 쪽으로 굴러요.
    const Vec2 input_dir = engine::normalized(move_input);
    dodge_direction_ = engine::length(input_dir) > 0.0f ? input_dir : direction_vector(facing_);
    facing_ = facing_from_vector(dodge_direction_, facing_);
    state_ = State::Dodging;
    dodge_elapsed_ = 0.0f;
}

void Mover::update_velocity(float dt, Vec2 move_input, bool run) {
    // 입력이 1보다 길면 1로 줄여요 (대각선 속도 보호)
    Vec2 input = move_input;
    if (engine::length(input) > 1.0f) {
        input = engine::normalized(input);
    }
    // 달리면 최고 속도가 더 높아요. 달리기를 멈추면 걷기 속도까지 가속도만큼 부드럽게 줄어요.
    const float top_speed = tuning_.max_speed * (run ? tuning_.run_multiplier : 1.0f);
    const Vec2 target = input * top_speed;
    const bool has_input = engine::length(input) > 0.0f;
    const float rate = has_input ? tuning_.acceleration : tuning_.deceleration;
    velocity_ = move_towards(velocity_, target, rate * dt);
}

void Mover::update_facing(Vec2 move_input) {
    // 바라보는 방향은 "누르는 키" 기준이에요. 벽에 막혀 멈춰도 누르는 쪽을 봐요.
    facing_ = facing_from_vector(move_input, facing_);
}

}  // namespace wind::game
