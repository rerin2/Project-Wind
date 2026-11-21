#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 쓰러질 때의 슬로모션이에요. (명세 9.5, 12b)
// - trigger()하면 시뮬레이션 시간 배수가 start_scale(0.3배)로 떨어졌다가,
//   seconds(1.5초) 동안 서서히 1배로 돌아와요. 처음엔 오래 느리고 끝에서 빨리 돌아와요(InQuad).
// - 시간은 "진짜 시간"(고정 박자 길이)으로 흘러요. 그래야 느려진 동안에도 정해진 때에 끝나요.
// - 히트스톱처럼 시간 배수로만 시뮬레이션에 전해요. 위치를 몰래 바꾸지 않아요 (명세 3.2).
// SDL을 몰라서 창 없이 테스트할 수 있어요. 값의 이유는 docs/GAMEPLAY.md "Hero falling" 절에 있어요.

namespace wind::game {

// 슬로모션 숫자 (개발자와 정함, 2026-11-22)
struct SlowMotionTuning {
    float start_scale = 0.3f;  // 쓰러진 순간 시뮬레이션 빠르기 (0.3배)
    float seconds = 1.5f;      // 1배로 돌아오기까지 (진짜 시간, 초)
};

class SlowMotion {
public:
    explicit SlowMotion(SlowMotionTuning tuning = {});

    // 처음부터 다시 느려지기 시작해요
    void trigger();
    // 진짜 시간이 흘렀어요 (고정 박자마다 한 번, 박자 길이로)
    void update(float real_dt_seconds);

    // 시뮬레이션 시간 배수: 느려지는 중이면 start_scale~1, 아니면 1
    [[nodiscard]] float time_scale() const;
    [[nodiscard]] bool active() const { return elapsed_ < tuning_.seconds; }

private:
    SlowMotionTuning tuning_;
    float elapsed_;  // trigger 뒤 흐른 진짜 시간 (처음엔 "이미 끝남")
};

}  // namespace wind::game
