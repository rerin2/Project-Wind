// 이 파일은 무엇을 하나요?
// 빛의 깜빡임과 숨쉬기를 사인(sin) 물결로 계산하는 곳이에요.
// 사인 물결은 -1과 1 사이를 부드럽게 오가는 모양이라, 0~1로 바꿔서 써요.

#include "engine/light/light_animation.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::engine {
namespace {

// 깜빡임에 쓰는 두 물결의 빠르기(라디안/초). 서로 나누어떨어지지 않아서 반복이 잘 안 느껴져요.
constexpr double kFlickerFastA = 9.1;
constexpr double kFlickerFastB = 15.7;
// 깜빡일 때 크기가 줄어드는 최대 비율 (밝기보다 훨씬 조금만 흔들어요)
constexpr float kFlickerRadiusShare = 0.12f;

// 깜빡임 물결: 0(가장 밝음) ~ 1(가장 어두움)
float flicker_wave(const Light& light, double t) {
    const double phase = static_cast<double>(light.phase);
    const double wave = 0.6 * std::sin(t * kFlickerFastA + phase) +
                        0.4 * std::sin(t * kFlickerFastB + phase * 2.3);
    return static_cast<float>(0.5 + 0.5 * wave);
}

}  // namespace

float light_strength(const Light& light, double time_seconds) {
    float strength = light.intensity;
    if (light.flicker > 0.0f) {
        strength *= 1.0f - light.flicker * flicker_wave(light, time_seconds);
    }
    if (light.pulse_depth > 0.0f && light.pulse_frequency > 0.0f) {
        const double turn = 2.0 * std::numbers::pi * static_cast<double>(light.pulse_frequency) *
                                time_seconds + static_cast<double>(light.phase);
        const float wave = static_cast<float>(0.5 + 0.5 * std::sin(turn));
        strength *= 1.0f - light.pulse_depth * wave;
    }
    return std::clamp(strength, 0.0f, 1.0f);
}

float light_radius_scale(const Light& light, double time_seconds) {
    if (light.flicker <= 0.0f) {
        return 1.0f;
    }
    return 1.0f - light.flicker * kFlickerRadiusShare * flicker_wave(light, time_seconds);
}

}  // namespace wind::engine
