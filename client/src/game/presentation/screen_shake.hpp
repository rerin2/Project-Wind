#pragma once

// 이 파일은 무엇을 하나요?
// 화면 흔들림이에요. "트라우마" 방식을 써요. (명세 6.7)
// - 맞을 때마다 트라우마(0~1)가 쌓이고, 시간이 지나면 줄어요.
// - 실제로 흔들리는 양은 트라우마의 제곱에 비례해요. 그래서 약한 타격은 거의 안 흔들리고,
//   콤보 마무리처럼 센 타격이나 연달아 맞을 때만 크게 흔들려요.
// - 흔들림 방향은 두 개의 사인파를 섞어 만들어요. 주사위를 쓰지 않아 부드럽고 테스트하기 쉬워요.
// - 세기 설정(접근성)이 0이면 아무리 맞아도 0이에요.
// 보여 주기 전용: 카메라가 그리는 위치만 옮기고, 게임 결과는 바꾸지 않아요.

#include "engine/core/math.hpp"

namespace wind::game {

// 흔들림 숫자. 이유는 docs/VISUALS.md "Hit feel" 절에 있어요.
struct ScreenShakeTuning {
    float trauma_per_shake = 2.0f;  // 공격 데이터의 shake 값에 곱해서 트라우마로 더해요
    float decay_per_second = 1.6f;  // 1초에 줄어드는 트라우마 (1 -> 0까지 약 0.6초)
    float max_offset = 6.0f;        // 트라우마 1일 때 가장 크게 흔들리는 거리 (픽셀)
};

class ScreenShake {
public:
    explicit ScreenShake(ScreenShakeTuning tuning = {});

    // 맞은 공격의 흔들림 세기(0~1)를 더해요. 트라우마는 1을 넘지 않아요.
    void add(float shake);
    // 화면 한 장마다: 트라우마를 줄이고 흔들림 시계를 돌려요.
    void update(float dt_seconds);
    // 접근성 세기 0~1 (범위 밖이면 잘라요)
    void set_strength(float strength);

    // 지금 카메라를 옮길 거리 (픽셀). 세기 0이거나 트라우마 0이면 (0, 0)
    [[nodiscard]] engine::Vec2 offset() const;
    [[nodiscard]] float trauma() const { return trauma_; }

private:
    ScreenShakeTuning tuning_;
    float trauma_ = 0.0f;
    float strength_ = 1.0f;
    float clock_ = 0.0f;  // 흔들림 파도 시계 (초)
};

}  // namespace wind::game
