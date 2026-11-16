// 이 파일은 무엇을 하나요?
// 피해 숫자(DamageNumbers)의 움직임을 숙제 검사(테스트)해요.
// - 맞은 곳에서 위로 튀어 오르고(치명타는 더 높이), 꼭대기에서 머물러요
// - 수명의 60%까지는 진하고, 그 뒤 흐려지다 사라져요 (치명타는 더 오래)
// - 연달아 나오면 좌우로 비키고, 너무 많으면 가장 오래된 것부터 지워요
#include "game/presentation/damage_numbers.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::DamageNumbers;

constexpr float kDt = 1.0f / 60.0f;

void run(DamageNumbers& numbers, float seconds) {
    for (float t = 0.0f; t < seconds; t += kDt) numbers.update(kDt);
}

TEST(DamageNumbers, RisesThenHolds) {
    DamageNumbers numbers;
    numbers.spawn({100.0f, 200.0f}, 14, false);
    const auto& n = numbers.numbers().front();
    EXPECT_FLOAT_EQ(numbers.position(n).y, 200.0f);
    run(numbers, 0.3f);  // 오르기(0.245초)가 끝난 뒤
    EXPECT_NEAR(numbers.position(numbers.numbers().front()).y, 186.0f, 1e-3f);  // 14픽셀 위
    EXPECT_FLOAT_EQ(numbers.alpha(numbers.numbers().front()), 1.0f);
}

TEST(DamageNumbers, FadesAndDisappears) {
    DamageNumbers numbers;
    numbers.spawn({0.0f, 0.0f}, 14, false);
    run(numbers, 0.55f);
    ASSERT_EQ(numbers.numbers().size(), 1u);
    const float a = numbers.alpha(numbers.numbers().front());
    EXPECT_GT(a, 0.0f);
    EXPECT_LT(a, 1.0f);
    run(numbers, 0.2f);  // 0.75초 > 수명 0.7초
    EXPECT_TRUE(numbers.numbers().empty());
}

TEST(DamageNumbers, CriticalRisesHigherAndLivesLonger) {
    DamageNumbers numbers;
    numbers.spawn({0.0f, 0.0f}, 21, true);
    run(numbers, 0.75f);  // 보통 숫자라면 사라졌을 때
    ASSERT_EQ(numbers.numbers().size(), 1u);
    EXPECT_NEAR(numbers.position(numbers.numbers().front()).y, -20.0f, 1e-3f);
    EXPECT_TRUE(numbers.numbers().front().critical);
}

TEST(DamageNumbers, AlternatesSidesAndCapsCount) {
    DamageNumbers numbers;
    numbers.spawn({0.0f, 0.0f}, 1, false);
    numbers.spawn({0.0f, 0.0f}, 2, false);
    numbers.spawn({0.0f, 0.0f}, 3, false);
    EXPECT_FLOAT_EQ(numbers.position(numbers.numbers()[0]).x, 0.0f);
    EXPECT_FLOAT_EQ(numbers.position(numbers.numbers()[1]).x, 5.0f);
    EXPECT_FLOAT_EQ(numbers.position(numbers.numbers()[2]).x, -5.0f);
    for (int i = 0; i < 40; ++i) numbers.spawn({0.0f, 0.0f}, 100 + i, false);
    EXPECT_EQ(numbers.numbers().size(), 32u);
    EXPECT_EQ(numbers.numbers().back().amount, 139);  // 최신은 남고 오래된 것이 지워져요
}

}  // namespace
