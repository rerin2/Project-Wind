// HUD 바 자리·길이 계산 테스트 (12a)

#include "game/ui/hud_bar_layout.hpp"

#include <gtest/gtest.h>

#include <limits>

using wind::game::HudBarRow;
using wind::game::hud_bar_inner_width;
using wind::game::hud_bar_y;

// 순서는 위에서부터 HP·MP·스태미나, 줄 사이는 2픽셀 (명세 13.2)
TEST(HudBarLayout, RowsGoHealthManaStaminaFromTop) {
    EXPECT_FLOAT_EQ(hud_bar_y(HudBarRow::Health), 12.0f);
    EXPECT_FLOAT_EQ(hud_bar_y(HudBarRow::Mana), 21.0f);
    EXPECT_FLOAT_EQ(hud_bar_y(HudBarRow::Stamina), 30.0f);
}

// 최대값이 늘면 바가 길어져요: HP 100 → 80, MP 50 → 40, HP 150 → 120
TEST(HudBarLayout, WidthGrowsWithMax) {
    EXPECT_FLOAT_EQ(hud_bar_inner_width(100.0f), 80.0f);
    EXPECT_FLOAT_EQ(hud_bar_inner_width(50.0f), 40.0f);
    EXPECT_FLOAT_EQ(hud_bar_inner_width(150.0f), 120.0f);
    EXPECT_LT(hud_bar_inner_width(100.0f), hud_bar_inner_width(110.0f));  // 생명력을 올리면 길어짐
}

// 이상한 값이 와도 8~200 안이에요
TEST(HudBarLayout, WidthIsClampedForOddValues) {
    EXPECT_FLOAT_EQ(hud_bar_inner_width(0.0f), 8.0f);
    EXPECT_FLOAT_EQ(hud_bar_inner_width(-20.0f), 8.0f);
    EXPECT_FLOAT_EQ(hud_bar_inner_width(99999.0f), 200.0f);
    EXPECT_FLOAT_EQ(hud_bar_inner_width(std::numeric_limits<float>::quiet_NaN()), 8.0f);
    EXPECT_FLOAT_EQ(hud_bar_inner_width(std::numeric_limits<float>::infinity()), 8.0f);
}

// 반 픽셀 길이가 나오지 않아요 (픽셀 아트가 번지지 않게)
TEST(HudBarLayout, WidthIsWholePixels) {
    EXPECT_FLOAT_EQ(hud_bar_inner_width(51.0f), 41.0f);  // 40.8 → 41
    EXPECT_FLOAT_EQ(hud_bar_inner_width(52.0f), 42.0f);  // 41.6 → 42
}
