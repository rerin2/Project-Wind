// 이 파일은 무엇을 하나요?
// 아이리스 가리개 계산을 숙제 검사(테스트)해요.
// - 처음(덮임 0)에는 아무것도 안 가려요
// - 끝(덮임 1)에는 화면을 빈틈없이 다 가려요
// - 중간에는 가운데 구멍만 남고, 가린 넓이가 "화면 - 동그라미" 넓이와 비슷해요
#include "engine/fx/iris_mask.hpp"

#include <gtest/gtest.h>

#include <numbers>

namespace {

using wind::engine::iris_cover_rects;
using wind::engine::iris_radius;
using wind::engine::RectF;
using wind::engine::Vec2;

constexpr Vec2 kScreen{640.0f, 360.0f};

float covered_area(const std::vector<RectF>& rects) {
    float area = 0.0f;
    for (const auto& r : rects) {
        area += r.w * r.h;
    }
    return area;
}

bool covers_point(const std::vector<RectF>& rects, Vec2 p) {
    for (const auto& r : rects) {
        if (p.x >= r.x && p.x < r.right() && p.y >= r.y && p.y < r.bottom()) {
            return true;
        }
    }
    return false;
}

// 가운데가 화면 왼쪽에 치우쳐 있어도, 덮임 0이면 구멍이 가장 먼 모서리까지 닿아요.
TEST(IrisMask, StartCoversNothing) {
    const Vec2 center{100.0f, 200.0f};
    const float radius = iris_radius(center, kScreen, 0.0f);
    EXPECT_GE(radius * radius, 540.0f * 540.0f + 200.0f * 200.0f - 1.0f);
    EXPECT_TRUE(iris_cover_rects(center, radius, kScreen).empty());
}

TEST(IrisMask, EndCoversEverything) {
    const Vec2 center{320.5f, 180.25f};
    const float radius = iris_radius(center, kScreen, 1.0f);
    EXPECT_FLOAT_EQ(radius, 0.0f);
    const auto rects = iris_cover_rects(center, radius, kScreen);
    EXPECT_FLOAT_EQ(covered_area(rects), kScreen.x * kScreen.y);
    EXPECT_TRUE(covers_point(rects, {320.0f, 180.0f}));
}

// 반지름 100 구멍: 가운데는 보이고, 구멍 밖(모서리, 가운데에서 120픽셀 오른쪽)은 가려져요.
TEST(IrisMask, MiddleLeavesARoundHole) {
    const Vec2 center{320.0f, 180.0f};
    const auto rects = iris_cover_rects(center, 100.0f, kScreen);
    EXPECT_FALSE(covers_point(rects, center));
    EXPECT_FALSE(covers_point(rects, {390.0f, 180.0f}));
    EXPECT_TRUE(covers_point(rects, {440.0f, 180.0f}));
    EXPECT_TRUE(covers_point(rects, {0.0f, 0.0f}));
    EXPECT_TRUE(covers_point(rects, {400.0f, 260.0f}));  // 대각선 방향 113픽셀: 구멍 밖
    const float hole = std::numbers::pi_v<float> * 100.0f * 100.0f;
    EXPECT_NEAR(covered_area(rects), kScreen.x * kScreen.y - hole, hole * 0.02f);
}

}  // namespace
