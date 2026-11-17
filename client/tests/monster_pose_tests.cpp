// 이 파일은 무엇을 하나요?
// 슬라임 몸 모양 계산(monster_pose)을 숙제 검사(테스트)해요. (11a)
// - 쉬는 중에는 뜨지 않고, 숨쉬기 때문에 가로·세로가 반대로 조금 바뀌어요
// - 걷는 중에는 한 뜀의 가운데에서 가장 높이 뜨고 위로 길쭉해져요. 땅에 닿을 때는 납작해요
// - 뜀 세기는 한 번에 확 바뀌지 않고 부드럽게 바뀌어요
#include "game/presentation/monster_pose.hpp"

#include <gtest/gtest.h>

namespace {

using wind::game::approach_hop_blend;
using wind::game::slime_pose;

constexpr float kHopLength = 14.0f;  // monster_pose.cpp와 같은 값: 한 번 뛸 때 가는 거리

TEST(MonsterPose, IdleBreathesWithoutLifting) {
    const auto a = slime_pose(0.35f, 0.0f, 0.0f, 0.0f);  // 숨 1/4쯤: 가로로 퍼짐
    EXPECT_FLOAT_EQ(a.lift, 0.0f);
    EXPECT_GT(a.scale_x, 1.0f);
    EXPECT_LT(a.scale_y, 1.0f);
    EXPECT_NEAR(a.scale_x - 1.0f, 1.0f - a.scale_y, 1e-5f);  // 퍼진 만큼 낮아져요
    EXPECT_LE(a.scale_x, 1.06f + 1e-5f);                    // 6%보다 크게 바뀌지 않아요
}

TEST(MonsterPose, HopPeaksMidwayAndSquashesOnLanding) {
    const auto peak = slime_pose(0.0f, 0.0f, kHopLength * 0.5f, 1.0f);  // 한 뜀의 가운데
    EXPECT_NEAR(peak.lift, 3.0f, 1e-4f);                                 // 가장 높이 (3픽셀)
    EXPECT_GT(peak.scale_y, 1.0f);                                       // 위로 길쭉
    EXPECT_LT(peak.scale_x, 1.0f);
    const auto land = slime_pose(0.0f, 0.0f, kHopLength * 2.0f, 1.0f);  // 땅에 닿는 순간
    EXPECT_NEAR(land.lift, 0.0f, 1e-4f);
    EXPECT_GT(land.scale_x, 1.0f);                                       // 납작
    EXPECT_LT(land.scale_y, 1.0f);
}

TEST(MonsterPose, NoHopWhenBlendIsZero) {
    const auto pose = slime_pose(0.0f, 0.0f, kHopLength * 0.5f, 0.0f);  // 걸은 거리가 있어도 세기 0이면
    EXPECT_FLOAT_EQ(pose.lift, 0.0f);
}

TEST(MonsterPose, HopBlendMovesSmoothly) {
    float blend = 0.0f;
    blend = approach_hop_blend(blend, true, 1.0f / 60.0f);  // 걷기 시작: 한 화면에 조금만
    EXPECT_GT(blend, 0.0f);
    EXPECT_LT(blend, 0.5f);
    for (int i = 0; i < 60; ++i) blend = approach_hop_blend(blend, true, 1.0f / 60.0f);
    EXPECT_FLOAT_EQ(blend, 1.0f);                             // 1초면 다 올라가고 넘치지 않아요
    for (int i = 0; i < 60; ++i) blend = approach_hop_blend(blend, false, 1.0f / 60.0f);
    EXPECT_FLOAT_EQ(blend, 0.0f);                             // 멈추면 0으로
}

}  // namespace
