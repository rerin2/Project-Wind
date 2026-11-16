#pragma once

// 이 파일은 무엇을 하나요?
// "용수철 흔들림" 넉백이에요. (명세 6.7 넉백, 10a)
// 땅에 박힌 물건(허수아비)은 칸 밖으로 밀려나면 안 돼서, 몸통만 맞은 쪽으로 밀렸다가
// 용수철처럼 제자리로 돌아와요. 밀린 거리(offset)는 진짜 게임 값이라 맞는 상자도 같이 움직여요.
// - push: 맞은 방향으로 속도를 줘요 (넉백 값 = 픽셀/초)
// - step: 용수철 힘(제자리로 당김)과 마찰(흔들림이 줄어듦)로 한 박자 움직여요
// - 아무리 세게 맞아도 max_offset(8픽셀)보다 멀리 가지 않아요
// 진짜로 밀려나 걸어가는 넉백은 몬스터(11단계)에서 Mover로 해요.

#include "engine/core/math.hpp"

namespace wind::game {

// 용수철 숫자. 이유는 docs/GAMEPLAY.md "Knockback" 절에 있어요.
struct KnockSwayTuning {
    float stiffness = 300.0f;   // 제자리로 당기는 힘 (1/초²). 클수록 빨리 돌아와요
    float damping = 14.0f;      // 마찰 (1/초). 클수록 덜 출렁여요
    float max_offset = 8.0f;    // 가장 멀리 밀리는 거리 (픽셀)
    float knockback_scale = 1.0f;  // 넉백 값에 곱해서 처음 속도로 써요
};

class KnockSway {
public:
    explicit KnockSway(KnockSwayTuning tuning = {});

    // direction(길이 1) 쪽으로 knockback(픽셀/초)만큼 밀어요. 0 이하면 아무 일 없어요.
    void push(engine::Vec2 direction, float knockback);
    // 한 박자 움직여요. dt가 0 이하면(히트스톱) 그대로 멈춰 있어요.
    void step(float dt_seconds);

    // 제자리에서 밀린 거리 (픽셀)
    [[nodiscard]] engine::Vec2 offset() const { return offset_; }
    [[nodiscard]] engine::Vec2 velocity() const { return velocity_; }
    [[nodiscard]] bool resting() const;

private:
    KnockSwayTuning tuning_;
    engine::Vec2 offset_;
    engine::Vec2 velocity_;
};

}  // namespace wind::game
