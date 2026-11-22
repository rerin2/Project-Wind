// 이 파일은 무엇을 하나요?
// 능력치 곡선 검사 (13a): 점 위의 값, 점 사이 직선, 양 끝 밖은 평평하게.
#include "wind/progression/stat_curve.hpp"

#include <gtest/gtest.h>

using wind::progression::StatCurve;

namespace {

StatCurve hp_curve() {  // 명세 8.2 최대 HP 곡선
    return StatCurve({{10, 100.0}, {20, 190.0}, {30, 280.0}, {40, 360.0}, {60, 450.0}});
}

}  // namespace

TEST(StatCurve, ValueOnPoints) {
    const auto curve = hp_curve();
    EXPECT_DOUBLE_EQ(curve.value_at(10), 100.0);
    EXPECT_DOUBLE_EQ(curve.value_at(40), 360.0);
    EXPECT_DOUBLE_EQ(curve.value_at(60), 450.0);
}

TEST(StatCurve, StraightLineBetweenPoints) {
    const auto curve = hp_curve();
    EXPECT_DOUBLE_EQ(curve.value_at(11), 109.0);   // 1당 9
    EXPECT_DOUBLE_EQ(curve.value_at(15), 145.0);
    EXPECT_DOUBLE_EQ(curve.value_at(50), 405.0);   // 40~60은 1당 4.5 (소프트 캡)
}

TEST(StatCurve, FlatOutsidePoints) {
    const auto curve = hp_curve();
    EXPECT_DOUBLE_EQ(curve.value_at(1), 100.0);    // 첫 점보다 낮으면 첫 값
    EXPECT_DOUBLE_EQ(curve.value_at(99), 450.0);   // 마지막 점보다 높으면 마지막 값
}

TEST(StatCurve, EmptyCurveIsZero) {
    EXPECT_DOUBLE_EQ(StatCurve{}.value_at(10), 0.0);
}

TEST(StatCurve, SinglePointIsFlat) {
    const StatCurve curve({{30, 7.0}});
    EXPECT_DOUBLE_EQ(curve.value_at(1), 7.0);
    EXPECT_DOUBLE_EQ(curve.value_at(60), 7.0);
}
