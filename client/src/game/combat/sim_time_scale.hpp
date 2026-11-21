#pragma once

// 이 파일은 무엇을 하나요?
// 필드 시뮬레이션의 "시간 배수"를 한 곳에서 정해요. (명세 3.2, 12b)
// - 히트스톱(맞는 순간 멈칫, 10a): 멈춘 동안 0
// - 슬로모션(주인공이 쓰러질 때, 12b): 0.3배에서 1배로 서서히
// 두 배수를 곱해서 시뮬레이션에 넘겨요 (시뮬레이션 시간 = 박자 × 배수).
// 둘 다 진짜 시간(박자 길이)으로 줄어들어서, 느려지거나 멈춘 동안에도 정해진 때에 끝나요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "game/combat/hit_stop.hpp"
#include "game/combat/slow_motion.hpp"

namespace wind::game {

class SimTimeScale {
public:
    explicit SimTimeScale(SlowMotionTuning slow_motion = {});

    // 지금 시뮬레이션 시간 배수 (0~1)
    [[nodiscard]] float scale() const { return hit_stop_.time_scale() * slow_motion_.time_scale(); }

    // 박자 계산이 끝난 뒤 한 번: 진짜 시간을 흘리고, 이번 박자의 일을 반영해요.
    // hitstop: 이번에 맞은 일 중 가장 긴 멈칫 (초, 없으면 0), hero_fell: 주인공이 이번에 쓰러졌나
    void update(float real_dt_seconds, float hitstop, bool hero_fell);

    [[nodiscard]] const HitStop& hit_stop() const { return hit_stop_; }
    [[nodiscard]] const SlowMotion& slow_motion() const { return slow_motion_; }

private:
    HitStop hit_stop_;        // 멈칫 (10a)
    SlowMotion slow_motion_;  // 쓰러질 때 느리게 (12b)
};

}  // namespace wind::game
