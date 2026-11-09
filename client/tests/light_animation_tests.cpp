// 이 파일은 무엇을 하나요?
// 빛의 깜빡임과 숨쉬기 계산이 정해진 범위 안에서 움직이는지 숙제 검사(테스트)를 해요.
#include "engine/light/light_animation.hpp"

#include <gtest/gtest.h>

#include <algorithm>

namespace {

using wind::engine::Light;
using wind::engine::light_radius_scale;
using wind::engine::light_strength;

// 깜빡임·숨쉬기가 없으면 밝기는 언제나 intensity 그대로, 크기 배율도 1이에요.
TEST(LightAnimation, SteadyLightDoesNotChange) {
    Light light;
    light.intensity = 0.7f;
    for (double t : {0.0, 0.3, 5.0, 100.0}) {
        EXPECT_FLOAT_EQ(light_strength(light, t), 0.7f);
        EXPECT_FLOAT_EQ(light_radius_scale(light, t), 1.0f);
    }
}

// 깜빡임 0.35: 밝기는 1 - 0.35 ~ 1 사이에서 흔들리고, 실제로 바뀌어요.
TEST(LightAnimation, FlickerStaysInRangeAndMoves) {
    Light light;
    light.flicker = 0.35f;
    float lowest = 1.0f;
    float highest = 0.0f;
    for (int i = 0; i < 600; ++i) {
        const float s = light_strength(light, i / 60.0);
        lowest = std::min(lowest, s);
        highest = std::max(highest, s);
        EXPECT_GE(light_radius_scale(light, i / 60.0), 1.0f - 0.35f * 0.12f - 1e-5f);
    }
    EXPECT_GE(lowest, 0.65f - 1e-5f);
    EXPECT_LE(highest, 1.0f);
    EXPECT_GT(highest - lowest, 0.1f);  // 진짜로 깜빡여요
}

// 숨쉬기 1초에 0.5번, 세기 0.5: 1초 간격으로 가장 밝음과 가장 어두움(0.5)을 오가요.
TEST(LightAnimation, PulseFollowsFrequency) {
    Light light;
    light.pulse_frequency = 0.5f;
    light.pulse_depth = 0.5f;
    // sin이 0인 시각(t=0) -> 물결 0.5 -> 밝기 0.75
    EXPECT_NEAR(light_strength(light, 0.0), 0.75f, 1e-4f);
    // t=0.5초 -> sin = 1 -> 가장 어두움 0.5
    EXPECT_NEAR(light_strength(light, 0.5), 0.5f, 1e-4f);
    // t=1.5초 -> sin = -1 -> 가장 밝음 1
    EXPECT_NEAR(light_strength(light, 1.5), 1.0f, 1e-4f);
}

// 출발점(phase)이 다르면 같은 시각에 다른 밝기예요 (횃불끼리 똑같이 깜빡이지 않게).
TEST(LightAnimation, PhaseDesynchronizesLights) {
    Light a;
    a.flicker = 0.4f;
    Light b = a;
    b.phase = 1.7f;
    EXPECT_NE(light_strength(a, 2.0), light_strength(b, 2.0));
}

}  // namespace
