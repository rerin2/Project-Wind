// 이 파일은 무엇을 하나요?
// 하루 색 곡선에서 어떤 시각의 색을 맞게 섞는지 숙제 검사(테스트)를 해요.
// 특히 자정을 넘어가는 구간(21시 -> 다음 날 0시)이 부드럽게 이어지는지 봐요.
#include "wind/world/day_cycle.hpp"

#include <gtest/gtest.h>

namespace {

using wind::world::DayCycle;
using wind::world::Rgba;
using wind::world::sample_day_cycle;

// 6시: 검정(밤 1) -> 12시: 흰색(밤 0) -> 18시: 빨강(밤 0.5)
DayCycle make_cycle() {
    DayCycle cycle;
    cycle.keys = {{6.0f, {0, 0, 0, 255}, 1.0f},
                  {12.0f, {255, 255, 255, 255}, 0.0f},
                  {18.0f, {255, 0, 0, 255}, 0.5f}};
    return cycle;
}

// 점 위의 시각은 그 점의 색 그대로
TEST(DayCycle, ExactKeysReturnKeyColor) {
    const auto cycle = make_cycle();
    EXPECT_EQ(sample_day_cycle(cycle, 12.0f).ambient, (Rgba{255, 255, 255, 255}));
    EXPECT_FLOAT_EQ(sample_day_cycle(cycle, 6.0f).night, 1.0f);
}

// 9시 = 6시와 12시의 딱 가운데 -> 회색, 밤 0.5
TEST(DayCycle, BlendsBetweenKeys) {
    const auto s = sample_day_cycle(make_cycle(), 9.0f);
    EXPECT_EQ(s.ambient.r, 128);
    EXPECT_EQ(s.ambient.g, 128);
    EXPECT_FLOAT_EQ(s.night, 0.5f);
}

// 자정 넘기: 18시(빨강) -> 다음 날 6시(검정) 구간은 12시간. 0시는 그 가운데예요.
TEST(DayCycle, WrapsAroundMidnight) {
    const auto midnight = sample_day_cycle(make_cycle(), 0.0f);
    EXPECT_EQ(midnight.ambient.r, 128);
    EXPECT_EQ(midnight.ambient.g, 0);
    EXPECT_FLOAT_EQ(midnight.night, 0.75f);
    // 21시와 3시도 같은 구간 안에서 이어져요.
    EXPECT_EQ(sample_day_cycle(make_cycle(), 21.0f).ambient.r, 191);
    EXPECT_EQ(sample_day_cycle(make_cycle(), 3.0f).ambient.r, 64);
}

// 24시를 넘거나 음수면 하루 단위로 감아요: 33시 = 9시, -15시 = 9시
TEST(DayCycle, WrapsHoursOutsideOneDay) {
    const auto cycle = make_cycle();
    EXPECT_EQ(sample_day_cycle(cycle, 33.0f).ambient, sample_day_cycle(cycle, 9.0f).ambient);
    EXPECT_EQ(sample_day_cycle(cycle, -15.0f).ambient, sample_day_cycle(cycle, 9.0f).ambient);
}

}  // namespace
