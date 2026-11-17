#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 맞았을 때의 몸 반응이에요. (명세 9.4, 6.7, 11c)
// - 경직(hit): 정해진 시간(기본 0.3초) 동안 움직이거나 공격·구르기·마술을 못 해요.
// - 넉백: 맞은 방향으로 밀려나요. 처음 빠르기(공격 데이터의 knockback)에서 경직이 끝날 때 0이 되도록
//   일정하게 느려져요. 그래서 밀리는 거리 = 빠르기 × 경직 시간 ÷ 2 (160px/s, 0.3초면 24픽셀).
//   실제로 벽에 부딪히며 옮기는 일은 PlayerMotion이 Mover로 해요 (벽을 따라 미끄러져요).
// - 무적: 맞은 뒤 정해진 시간(기본 0.6초) 동안 다시 맞지 않아요. 화면은 이 동안 깜빡여요.
// 게임 규칙이라 정해진 박자마다 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"

namespace wind::game {

// 피격 반응 숫자 모음. 값의 이유는 docs/GAMEPLAY.md "Hero getting hit" 절에 있어요.
struct HitTuning {
    float stagger_seconds = 0.3f;       // 경직 시간
    float invulnerable_seconds = 0.6f;  // 맞은 뒤 무적 시간 (명세 9.4 기본값)
    engine::RectF hurtbox{-7.0f, -30.0f, 14.0f, 30.0f};  // 발 기준 맞는 상자 (x, y = 왼쪽 위, 발에서)
};

class HitReaction {
public:
    explicit HitReaction(HitTuning tuning);

    // 맞았어요: 경직·무적을 처음부터 시작하고, direction(길이 1) 쪽으로 knockback(픽셀/초)만큼 밀기 시작해요.
    void start(engine::Vec2 direction, float knockback);
    // 한 박자: 시간을 줄이고, 이번 박자에 밀려날 거리(화살표)를 돌려줘요.
    [[nodiscard]] engine::Vec2 step(float dt_seconds);

    // 경직 중인가요? (이동·행동 막힘)
    [[nodiscard]] bool staggered() const { return stagger_left_ > 0.0f; }
    // 피격 무적 중인가요? (다시 맞지 않음, 화면은 깜빡임)
    [[nodiscard]] bool invulnerable() const { return invulnerable_left_ > 0.0f; }
    // 무적 남은 시간 (초, 깜빡임 박자용)
    [[nodiscard]] float invulnerable_left() const { return invulnerable_left_; }
    [[nodiscard]] const HitTuning& tuning() const { return tuning_; }

private:
    HitTuning tuning_;
    float stagger_left_ = 0.0f;      // 경직 남은 시간 (초)
    float invulnerable_left_ = 0.0f; // 무적 남은 시간 (초)
    engine::Vec2 push_direction_;    // 밀리는 방향 (길이 1)
    float push_speed_ = 0.0f;        // 맞은 순간 밀리는 빠르기 (픽셀/초)
};

}  // namespace wind::game
