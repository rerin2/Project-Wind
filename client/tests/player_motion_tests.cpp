// 이 파일은 무엇을 하나요?
// 주인공 몸 움직임(PlayerMotion)이 스태미나 규칙대로 달리고 구르는지 숙제 검사해요. (8단계 완료 조건)
// - Shift를 누르고 움직이면 1.5배 빠르게 달리고, 1초에 25씩 스태미나를 써요
// - 제자리에서 Shift만 누르면 스태미나를 안 써요
// - 구르기는 25를 쓰고, 25가 안 되면 구르지 않아요
// - 다 쓰면 지침: 25까지 차야 다시 달리고 구를 수 있어요
#include "game/entities/player_motion.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Vec2;
using wind::game::Mover;
using wind::game::MovementTuning;
using wind::game::PlayerMotion;
using wind::game::StaminaTuning;

constexpr double kDt = 1.0 / 60.0;
constexpr Vec2 kRight{1.0f, 0.0f};

// 벽이 없는 아주 넓은 맵 (오래 달려도 벽에 안 닿게)
wind::world::MapData open_map() {
    wind::world::MapData map;
    map.tile_size = 32;
    map.width = 400;
    map.height = 20;
    map.walkable.assign(static_cast<std::size_t>(400 * 20), 1);
    return map;
}

PlayerMotion make_player() {
    return PlayerMotion({100.0f, 300.0f}, wind::world::Direction::Right, MovementTuning{},
                        StaminaTuning{}, 0.15);
}

void run_for(PlayerMotion& player, int steps, Vec2 move, bool run, const wind::world::MapData& map) {
    for (int i = 0; i < steps; ++i) {
        player.step(kDt, move, run, map);
    }
}

TEST(PlayerMotion, RunningIsFasterAndCostsStamina) {
    const auto map = open_map();
    auto player = make_player();
    run_for(player, 60, kRight, true, map);  // 1초 달리기
    EXPECT_EQ(player.mover().state(), Mover::State::Running);
    EXPECT_NEAR(player.mover().speed(), 165.0f, 0.5f);
    EXPECT_NEAR(player.stamina().current(), 75.0f, 0.5f);  // 1초에 25
}

TEST(PlayerMotion, StandingWithShiftCostsNothing) {
    const auto map = open_map();
    auto player = make_player();
    run_for(player, 60, {0.0f, 0.0f}, true, map);
    EXPECT_EQ(player.mover().state(), Mover::State::Idle);
    EXPECT_FLOAT_EQ(player.stamina().current(), 100.0f);
}

TEST(PlayerMotion, DodgeCostsStaminaAndNeedsEnough) {
    const auto map = open_map();
    auto player = make_player();
    player.press_dodge();
    EXPECT_TRUE(player.step(kDt, kRight, false, map));
    EXPECT_NEAR(player.stamina().current(), 75.0f, 1e-3f);

    // 3초 달려서 거의 다 쓰면(지침) 버튼을 눌러도 구르지 않아요.
    run_for(player, 30, kRight, false, map);  // 구르기 끝나고 쉬는 시간도 지나게
    run_for(player, 200, kRight, true, map);
    EXPECT_TRUE(player.stamina().exhausted());
    player.press_dodge();
    EXPECT_FALSE(player.step(kDt, kRight, false, map));
}

// 지침이면 Shift를 눌러도 걷는 속도예요. 쉬어서 25가 되면 다시 달려요.
TEST(PlayerMotion, ExhaustedWalksUntilRecovered) {
    const auto map = open_map();
    auto player = make_player();
    // 1초에 25씩이라 4초(240박자) 만에 0이 돼요. 그 순간부터 시간을 세요.
    run_for(player, 240, kRight, true, map);
    // Shift를 계속 눌러도 지침이라 걷는 속도로 줄어요. (0이 된 뒤 30박자 = 0.5초, 아직 안 참)
    run_for(player, 30, kRight, true, map);
    EXPECT_TRUE(player.stamina().exhausted());
    EXPECT_EQ(player.mover().state(), Mover::State::Walking);
    EXPECT_NEAR(player.mover().speed(), 110.0f, 0.5f);

    // 걷는 동안에도 차요. 0.6초(36박자) 기다린 뒤 1초에 30씩 -> 25까지 약 50박자 더.
    run_for(player, 50, kRight, true, map);  // 0이 된 뒤 80박자: 약 22 -> 아직 지침
    EXPECT_TRUE(player.stamina().exhausted());
    run_for(player, 10, kRight, true, map);  // 90박자: 약 27 -> 다시 달릴 수 있어요
    EXPECT_FALSE(player.stamina().exhausted());
    run_for(player, 30, kRight, true, map);
    EXPECT_EQ(player.mover().state(), Mover::State::Running);
}

}  // namespace
