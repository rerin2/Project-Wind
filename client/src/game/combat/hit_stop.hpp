#pragma once

// 이 파일은 무엇을 하나요?
// 히트스톱(맞는 순간 "멈칫")이에요. (명세 3.2, 6.7)
// - 맞은 일이 생기면 그 공격의 hitstop(초)만큼 시뮬레이션 시간을 멈춰요.
// - 멈추는 방법은 딱 하나: 시뮬레이션에 넘기는 시간 배수(time_scale)를 0으로 해요.
//   그래서 연출이 몰래 위치를 바꾸는 일이 없어요. 시간이 0만큼 흐를 뿐이에요.
// - 멈춤 시간은 "진짜 시간"(고정 박자 길이)으로 줄어요. 멈춘 동안 또 맞으면 더 긴 쪽을 따라요(더하지 않아요).
// 필드 전체를 멈춰요(주인공·허수아비·빛 탄 모두). 혼자 하는 게임이라 공격자·대상만 따로 멈출 필요가 없어요.
// 하루 시간과 연출(파티클·흔들림)은 계속 흘러요.

namespace wind::game {

class HitStop {
public:
    // seconds만큼 멈추기 (지금 남은 시간보다 길 때만 늘려요)
    void trigger(float seconds);
    // 진짜 시간이 흘렀어요 (고정 박자마다 한 번, 박자 길이로)
    void update(float real_dt_seconds);

    // 시뮬레이션 시간 배수: 멈춤 중이면 0, 아니면 1
    [[nodiscard]] float time_scale() const { return active() ? 0.0f : 1.0f; }
    [[nodiscard]] bool active() const { return remaining_ > 0.0f; }
    [[nodiscard]] float remaining() const { return remaining_; }

private:
    float remaining_ = 0.0f;  // 남은 멈춤 시간 (초)
};

}  // namespace wind::game
