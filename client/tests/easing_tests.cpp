// 이 파일은 무엇을 하나요?
// 이징 공식(움직임의 느낌)이 맞게 계산되는지 숙제 검사(테스트)를 해요.
#include "engine/tween/easing.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>

namespace {

using wind::engine::Ease;
using wind::engine::ease;

constexpr std::array kAll{Ease::Linear,    Ease::InQuad,     Ease::OutQuad,
                          Ease::InOutQuad, Ease::InCubic,    Ease::OutCubic,
                          Ease::InOutCubic, Ease::OutBack,   Ease::OutElastic};

// 모든 공식은 시작이 0, 끝이 1이에요. 범위 밖 t는 잘려요.
TEST(Easing, EndpointsAreZeroAndOne) {
    for (const Ease e : kAll) {
        EXPECT_NEAR(ease(e, 0.0f), 0.0f, 1e-5f);
        EXPECT_NEAR(ease(e, 1.0f), 1.0f, 1e-5f);
        EXPECT_NEAR(ease(e, -3.0f), 0.0f, 1e-5f);
        EXPECT_NEAR(ease(e, 7.0f), 1.0f, 1e-5f);
    }
}

// 반쯤 갔을 때: In은 아직 덜 갔고, Out은 벌써 많이 갔고, InOut은 딱 절반
TEST(Easing, MidpointShapes) {
    EXPECT_FLOAT_EQ(ease(Ease::Linear, 0.5f), 0.5f);
    EXPECT_FLOAT_EQ(ease(Ease::InQuad, 0.5f), 0.25f);
    EXPECT_FLOAT_EQ(ease(Ease::OutQuad, 0.5f), 0.75f);
    EXPECT_FLOAT_EQ(ease(Ease::InOutQuad, 0.5f), 0.5f);
    EXPECT_FLOAT_EQ(ease(Ease::InCubic, 0.5f), 0.125f);
    EXPECT_FLOAT_EQ(ease(Ease::OutCubic, 0.5f), 0.875f);
}

// OutBack은 끝나기 전에 목표(1)를 살짝 넘어가요.
TEST(Easing, OutBackOvershoots) {
    float peak = 0.0f;
    for (int i = 0; i <= 100; ++i) {
        peak = std::max(peak, ease(Ease::OutBack, static_cast<float>(i) / 100.0f));
    }
    EXPECT_GT(peak, 1.05f);
    EXPECT_LT(peak, 1.2f);
}

// 자료 파일 이름을 읽을 수 있고, 모르는 이름은 거절해요.
TEST(Easing, ParsesNames) {
    EXPECT_EQ(wind::engine::parse_ease("out_back"), Ease::OutBack);
    EXPECT_EQ(wind::engine::parse_ease("linear"), Ease::Linear);
    EXPECT_FALSE(wind::engine::parse_ease("bouncy").has_value());
}

}  // namespace
