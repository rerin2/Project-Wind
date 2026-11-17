// 이 파일은 무엇을 하나요?
// 몸끼리 밀어내기 계산(separate_bodies)을 숙제 검사(테스트)해요. (명세 7.3, 11c)
// - 안 겹치면 밀지 않아요
// - 겹치면 서로 반대쪽으로, 무거운 쪽은 덜 밀려요
// - 한 박자에 max_step까지만 (힘 제한)
// - 완전히 같은 자리여도 늘 같은 방향(오른쪽)으로 떼어 놓아요
#include "game/physics/body_separation.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::BodyCircle;
using wind::game::separate_bodies;

TEST(BodySeparation, NoPushWhenApart) {
    const auto push = separate_bodies({{0.0f, 0.0f}, 7.0f, 1.0f}, {{20.0f, 0.0f}, 6.0f, 3.0f}, 5.0f);
    EXPECT_FLOAT_EQ(push.first.x, 0.0f);
    EXPECT_FLOAT_EQ(push.second.x, 0.0f);
}

TEST(BodySeparation, SplitsByWeight) {
    // 반지름 7 + 6 = 13, 거리 9 → 겹침 4. 주인공(1) : 슬라임(3) = 1 : 3
    const auto push = separate_bodies({{0.0f, 0.0f}, 7.0f, 1.0f}, {{9.0f, 0.0f}, 6.0f, 3.0f}, 100.0f);
    EXPECT_NEAR(push.first.x, -1.0f, 1e-5f);  // 주인공은 왼쪽으로 1
    EXPECT_NEAR(push.second.x, 3.0f, 1e-5f);  // 슬라임은 오른쪽으로 3
    EXPECT_NEAR(push.first.y, 0.0f, 1e-5f);
}

TEST(BodySeparation, LimitedPerStep) {
    const auto push = separate_bodies({{0.0f, 0.0f}, 7.0f, 1.0f}, {{0.0f, 3.0f}, 6.0f, 1.0f}, 2.0f);
    // 겹침 10이지만 한 박자에 2까지만, 반반씩
    EXPECT_NEAR(push.first.y, -1.0f, 1e-5f);
    EXPECT_NEAR(push.second.y, 1.0f, 1e-5f);
}

TEST(BodySeparation, SameSpotPushesRightAndZeroWeightStays) {
    const auto push = separate_bodies({{5.0f, 5.0f}, 7.0f, 1.0f}, {{5.0f, 5.0f}, 6.0f, 1.0f}, 4.0f);
    EXPECT_NEAR(push.first.x, -2.0f, 1e-5f);
    EXPECT_NEAR(push.second.x, 2.0f, 1e-5f);
    const auto wall = separate_bodies({{0.0f, 0.0f}, 7.0f, 0.0f}, {{4.0f, 0.0f}, 6.0f, 1.0f}, 100.0f);
    EXPECT_FLOAT_EQ(wall.first.x, 0.0f);       // 몫 0은 안 밀려요
    EXPECT_NEAR(wall.second.x, 9.0f, 1e-5f);   // 상대가 겹친 만큼 다 밀려요
}

}  // namespace
