// 이 파일은 무엇을 하나요?
// 히트스톱과 슬로모션 시계를 흘리고, 이번 박자에 생긴 일(맞음, 쓰러짐)을 반영하는 곳이에요.

#include "game/combat/sim_time_scale.hpp"

namespace wind::game {

SimTimeScale::SimTimeScale(SlowMotionTuning slow_motion) : slow_motion_(slow_motion) {}

void SimTimeScale::update(float real_dt_seconds, float hitstop, bool hero_fell) {
    // 1) 이미 걸려 있던 멈춤·느림을 진짜 시간만큼 줄여요
    hit_stop_.update(real_dt_seconds);
    slow_motion_.update(real_dt_seconds);
    // 2) 이번 박자에 누가 맞았으면 다음 박자부터 멈칫 (10a)
    hit_stop_.trigger(hitstop);
    // 3) 주인공이 쓰러졌으면 다음 박자부터 느려져요 (12b). 멈칫이 끝나면 느린 시간이 이어져요.
    if (hero_fell) {
        slow_motion_.trigger();
    }
}

}  // namespace wind::game
