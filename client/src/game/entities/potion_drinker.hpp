#pragma once

// 이 파일은 무엇을 하나요?
// 물약 마시기 시간표예요. (명세 8.5 상태, 10.2 사용, 14d, 개발자와 정함)
// - 마시기는 0.6초예요. 시작할 때 물약은 이미 가방에서 나갔어요 (FieldItems가 꺼내요).
// - 0.35초(꿀꺽)에 효과가 한 번 나와요: 그 박자에 step이 효과를 돌려주면 PlayerMotion이 HP·MP를 채워요.
// - 꿀꺽 전에 맞아서 cancel되면 효과는 없고 물약은 돌아오지 않아요 (소울라이크 긴장감).
// - 꿀꺽 뒤 남은 0.25초는 병을 내리는 시간이라, 그동안에도 공격·구르기는 못 해요.
// 게임 규칙이라 SDL을 몰라요.

#include "wind/items/item_def.hpp"

#include <optional>

namespace wind::game {

class PotionDrinker {
public:
    static constexpr float kDrinkSeconds = 0.6f;  // 마시는 동작 전체
    static constexpr float kGulpSeconds = 0.35f;  // 이때 효과가 나와요 (꿀꺽)

    // 마시기를 시작해요 (이미 마시는 중이면 처음부터 다시)
    void start(const items::UseEffect& effect);
    // 한 박자. 이번 박자에 꿀꺽 시점을 지났으면 효과를 돌려줘요 (마시기마다 한 번)
    std::optional<items::UseEffect> step(float dt_seconds);
    // 맞아서 끊겨요: 아직 꿀꺽 전이면 효과는 사라져요
    void cancel();

    [[nodiscard]] bool busy() const { return effect_.has_value(); }
    // 시작한 뒤 흐른 시간 (연출용: 병 기울이기)
    [[nodiscard]] float elapsed() const { return elapsed_; }
    // 이미 꿀꺽했나요? (연출용)
    [[nodiscard]] bool gulped() const { return gulped_; }

private:
    std::optional<items::UseEffect> effect_;  // 마시는 중인 물약의 효과 (없으면 안 마시는 중)
    float elapsed_ = 0.0f;
    bool gulped_ = false;
};

}  // namespace wind::game
