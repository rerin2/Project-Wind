// 이 파일은 무엇을 하나요?
// 트라우마를 쌓고 줄이고, 지금 화면을 얼마나 옮길지 계산하는 곳이에요.

#include "game/presentation/screen_shake.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

// 흔들림 파도: 서로 딱 맞아떨어지지 않는 두 빠르기를 섞어서 같은 무늬가 반복되지 않게 해요.
// 가로와 세로는 출발점(위상)을 다르게 해서 대각선으로만 흔들리지 않게 해요.
constexpr float kWaveHzA = 11.0f;
constexpr float kWaveHzB = 17.0f;
constexpr float kWeightA = 0.6f;  // 두 파도의 비율 (합이 1이라 결과는 -1~1)
constexpr float kWeightB = 0.4f;
constexpr float kPhaseY = 1.7f;   // 세로 파도의 출발점

// 두 사인파를 섞은 -1~1 사이 값
float wave(float t, float phase) {
    constexpr float kTau = 2.0f * std::numbers::pi_v<float>;
    return kWeightA * std::sin(kTau * kWaveHzA * t + phase) +
           kWeightB * std::sin(kTau * kWaveHzB * t + phase * 2.0f);
}

}  // namespace

ScreenShake::ScreenShake(ScreenShakeTuning tuning) : tuning_(tuning) {}

void ScreenShake::add(float shake) {
    if (!std::isfinite(shake) || shake <= 0.0f) {
        return;  // 흔들림 없는 공격
    }
    trauma_ = std::min(1.0f, trauma_ + shake * tuning_.trauma_per_shake);
}

void ScreenShake::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    clock_ += dt_seconds;
    trauma_ = std::max(0.0f, trauma_ - tuning_.decay_per_second * dt_seconds);  // 천천히 가라앉아요
}

void ScreenShake::set_strength(float strength) {
    strength_ = std::isfinite(strength) ? std::clamp(strength, 0.0f, 1.0f) : 1.0f;
}

engine::Vec2 ScreenShake::offset() const {
    // 거리 = 최대 거리 × 세기 설정 × 트라우마² (명세 6.7 "세기² 비례")
    const float amount = tuning_.max_offset * strength_ * trauma_ * trauma_;
    if (amount <= 0.0f) {
        return {};
    }
    return {amount * wave(clock_, 0.0f), amount * wave(clock_, kPhaseY)};
}

}  // namespace wind::game
