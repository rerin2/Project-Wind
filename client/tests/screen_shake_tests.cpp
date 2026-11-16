// 이 파일은 무엇을 하나요?
// 화면 흔들림(ScreenShake)을 숙제 검사(테스트)해요.
// - 흔들림 세기 설정을 0으로 하면 아무리 맞아도 흔들리지 않아요 (10단계 완료 조건)
// - 트라우마는 쌓이되 1을 넘지 않고, 시간이 지나면 0으로 가라앉아요
// - 흔들리는 거리는 트라우마²에 비례하고 최대 6픽셀을 넘지 않아요
#include "game/presentation/screen_shake.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

namespace {

using wind::game::ScreenShake;

constexpr float kDt = 1.0f / 60.0f;

// 1초 동안 매 화면 가장 크게 흔들린 거리
float max_offset_over(ScreenShake& shake, int frames) {
    float most = 0.0f;
    for (int i = 0; i < frames; ++i) {
        shake.update(kDt);
        const auto o = shake.offset();
        most = std::max({most, std::abs(o.x), std::abs(o.y)});
    }
    return most;
}

TEST(ScreenShake, ZeroStrengthNeverShakes) {
    ScreenShake shake;
    shake.set_strength(0.0f);
    for (int i = 0; i < 10; ++i) shake.add(1.0f);  // 아주 세게 여러 번
    EXPECT_GT(shake.trauma(), 0.0f);                 // 트라우마는 쌓여도
    EXPECT_FLOAT_EQ(max_offset_over(shake, 60), 0.0f);  // 화면은 그대로
}

TEST(ScreenShake, TraumaCapsAndDecays) {
    ScreenShake shake;
    shake.add(0.35f);  // 롱소드 마무리: 0.7
    EXPECT_NEAR(shake.trauma(), 0.7f, 1e-6f);
    shake.add(0.35f);
    EXPECT_FLOAT_EQ(shake.trauma(), 1.0f);  // 1을 넘지 않아요
    for (int i = 0; i < 40; ++i) shake.update(kDt);  // 0.67초
    EXPECT_FLOAT_EQ(shake.trauma(), 0.0f);
    EXPECT_FLOAT_EQ(shake.offset().x, 0.0f);
}

// 약한 타격(0.05 -> 트라우마 0.1)은 1픽셀도 안 흔들리고, 센 타격은 눈에 보이게, 최대 6픽셀
TEST(ScreenShake, OffsetGrowsWithTraumaSquared) {
    ScreenShake weak;
    weak.add(0.05f);
    EXPECT_LT(max_offset_over(weak, 5), 0.5f);

    ScreenShake strong;
    strong.add(1.0f);
    const float most = max_offset_over(strong, 5);
    EXPECT_GT(most, 2.0f);
    EXPECT_LE(most, 6.0f + 1e-4f);
}

TEST(ScreenShake, IgnoresBadValues) {
    ScreenShake shake;
    shake.add(-1.0f);
    shake.add(std::nanf(""));
    EXPECT_FLOAT_EQ(shake.trauma(), 0.0f);
    shake.set_strength(5.0f);  // 1로 잘려요
    shake.add(1.0f);
    EXPECT_LE(max_offset_over(shake, 10), 6.0f + 1e-4f);
}

}  // namespace
