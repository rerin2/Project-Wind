// 이 파일은 무엇을 하나요?
// 쓰러짐 연출 시계(DeathFx)를 숙제 검사해요. (12b)
// - 서 있으면 흑백 0, 눕기 0
// - 쓰러지면 흑백이 1.5초 동안 0 -> 1, 눕기는 0.35초 동안 0 -> 1 (줄지 않아요)
// - 다시 서면(새 맵) 처음으로
#include "game/presentation/death_fx.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::DeathFx;

constexpr float kDt = 1.0f / 60.0f;

TEST(DeathFx, StandingShowsNothing) {
    DeathFx fx;
    fx.update(kDt, false);
    EXPECT_FALSE(fx.active());
    EXPECT_FLOAT_EQ(fx.grayscale(), 0.0f);
    EXPECT_FLOAT_EQ(fx.lying(), 0.0f);
}

TEST(DeathFx, GraysAndLiesDownOverTime) {
    DeathFx fx;
    fx.update(0.0f, true);  // 쓰러진 첫 화면
    EXPECT_TRUE(fx.active());
    EXPECT_FLOAT_EQ(fx.grayscale(), 0.0f);
    EXPECT_FLOAT_EQ(fx.lying(), 0.0f);
    float gray = 0.0f;
    float lie = 0.0f;
    for (int i = 0; i < 120; ++i) {  // 2초
        fx.update(kDt, true);
        EXPECT_GE(fx.grayscale(), gray);  // 거꾸로 가지 않아요
        EXPECT_GE(fx.lying(), lie);
        gray = fx.grayscale();
        lie = fx.lying();
        if (i == 10) {  // 0.18초: 눕기는 절반 안 됨(InQuad), 흑백은 이미 시작
            EXPECT_LT(fx.lying(), 0.5f);
            EXPECT_GT(fx.grayscale(), 0.0f);
        }
        if (i == 30) EXPECT_FLOAT_EQ(fx.lying(), 1.0f);  // 0.52초: 다 누움
    }
    EXPECT_FLOAT_EQ(fx.grayscale(), 1.0f);  // 1.5초 넘음: 완전 흑백
}

TEST(DeathFx, StandingUpResets) {
    DeathFx fx;
    for (int i = 0; i < 60; ++i) fx.update(kDt, true);
    fx.update(kDt, false);  // 새 맵에서 다시 섬
    EXPECT_FALSE(fx.active());
    EXPECT_FLOAT_EQ(fx.seconds(), 0.0f);
    EXPECT_FLOAT_EQ(fx.grayscale(), 0.0f);
}

}  // namespace
