// 이 파일은 무엇을 하나요?
// 주인공 한 박자: 구르기·달리기를 스태미나 규칙에 맞게 허락하고, 쓴 만큼 스태미나를 줄여요.

#include "game/entities/player_motion.hpp"

#include "game/input/action.hpp"

namespace wind::game {

PlayerMotion::PlayerMotion(engine::Vec2 feet, world::Direction facing, MovementTuning movement,
                           StaminaTuning stamina, double buffer_seconds)
    : mover_(feet, facing, movement), stamina_(stamina), buffer_(buffer_seconds) {}

void PlayerMotion::press_dodge() {
    buffer_.press(Action::Dodge);
}

bool PlayerMotion::step(double dt_seconds, engine::Vec2 move, bool run_held,
                        const world::MapData& map) {
    const auto dt = static_cast<float>(dt_seconds);
    const float dodge_cost = stamina_.tuning().dodge_cost;

    // 1) 구르기: 구를 수 있고 스태미나가 충분할 때만 메모를 꺼내요.
    //    모자라면 메모는 남아 있다가(0.15초 안에 스태미나가 차면 그때 굴러요) 사라져요.
    const bool dodge =
        mover_.can_dodge() && stamina_.can_afford(dodge_cost) && buffer_.consume(Action::Dodge);
    // 2) 달리기: Shift + 방향키 + 지침 아님
    const bool run = run_held && engine::length(move) > 0.0f && stamina_.can_run();

    const bool started = mover_.step(dt_seconds, move, dodge, map, run);
    if (started) {
        stamina_.spend(dodge_cost);
    }
    if (mover_.state() == Mover::State::Running) {
        stamina_.spend(stamina_.tuning().run_cost_per_second * dt);
    }
    // 3) 이번 박자에 썼으면 기다리는 시간이 다시 시작돼서, 이 update는 채우지 않아요.
    stamina_.update(dt);
    buffer_.tick(dt_seconds);
    return started;
}

}  // namespace wind::game
