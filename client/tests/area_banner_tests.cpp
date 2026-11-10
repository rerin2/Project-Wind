// 이 파일은 무엇을 하나요?
// 지역 이름 배너를 숙제 검사(테스트)해요.
// - 나타나기(0.5초) -> 머물기(2.2초) -> 사라지기(0.7초) 순서로 투명도가 바뀌나
// - 나타날 때 위에서 내려오나
#include "game/ui/area_banner.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::AreaBanner;

TEST(AreaBanner, FadesInHoldsAndFadesOut) {
    AreaBanner banner;
    EXPECT_FALSE(banner.visible());
    banner.show("시험 초원");
    EXPECT_TRUE(banner.visible());
    EXPECT_FLOAT_EQ(banner.alpha(), 0.0f);
    EXPECT_FLOAT_EQ(banner.rise(), AreaBanner::kRisePixels);
    banner.update(0.25f);
    EXPECT_GT(banner.alpha(), 0.0f);
    EXPECT_LT(banner.alpha(), 1.0f);
    banner.update(0.25f);
    EXPECT_FLOAT_EQ(banner.alpha(), 1.0f);  // 다 나타남
    EXPECT_FLOAT_EQ(banner.rise(), 0.0f);
    banner.update(2.0f);
    EXPECT_FLOAT_EQ(banner.alpha(), 1.0f);  // 머무는 중
    banner.update(0.3f);                    // 머물기 끝 -> 사라지기 시작
    banner.update(0.35f);
    EXPECT_GT(banner.alpha(), 0.0f);
    EXPECT_LT(banner.alpha(), 1.0f);
    banner.update(0.4f);
    EXPECT_FALSE(banner.visible());
    EXPECT_EQ(banner.name(), "시험 초원");
}

// 보이는 중에 다른 이름을 보여 주면 처음부터 다시 시작해요. 빈 이름은 안 보여요.
TEST(AreaBanner, ShowRestartsAndEmptyHides) {
    AreaBanner banner;
    banner.show("a");
    banner.update(1.0f);
    banner.show("b");
    EXPECT_FLOAT_EQ(banner.alpha(), 0.0f);
    EXPECT_EQ(banner.name(), "b");
    banner.show("");
    EXPECT_FALSE(banner.visible());
}

}  // namespace
