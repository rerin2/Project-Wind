#pragma once

// 이 파일은 무엇을 하나요?
// SpellCaster = 마술 "시전 시간"을 재는 담당이에요. (명세 8.6)
// - 시작하면 cast_seconds(별빛 조각 0.25초) 동안 모아요.
// - 다 모은 박자에 "나갔다!"(step이 true)를 알려 줘요. 그때 빛 탄이 생겨요.
// - 지금은 빛 탄 하나를 똑바로 쏘는 마술만 쓸 수 있어요. 근접·별무리는 15c, 광선은 15d에서 해요.
// MP와 입력 버퍼, 방향은 PlayerMotion이 맡고, 이 담당은 시간만 봐요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "wind/combat/spell.hpp"

#include <optional>

namespace wind::game {

class SpellCaster {
public:
    explicit SpellCaster(std::optional<combat::Spell> spell = {});

    // 고른 마술을 바꿔요 (없으면 마술을 못 써요). 시전 중이면 취소돼요.
    void set_spell(std::optional<combat::Spell> spell);

    // 지금 시전을 시작할 수 있나요? (마술이 있고, 지금 움직일 수 있는 마술이고, 시전 중이 아님)
    // 못 쓰는 마술이면 PlayerMotion이 MP를 쓰지 않아요.
    [[nodiscard]] bool can_start() const;
    // 시전 시작 (can_start()가 false면 아무 일도 없고 false)
    bool start();
    // 시전을 바로 멈춰요 (맞았을 때 등). 마술은 나가지 않아요.
    void cancel();
    // 한 박자 흘려보내요. 이번 박자에 시전이 끝나 마술이 나갔으면 true.
    bool step(float dt_seconds);

    [[nodiscard]] bool busy() const { return casting_; }
    // 모은 정도 0~1 (연출용: 손끝 빛이 커져요)
    [[nodiscard]] float progress() const;
    // 고른 마술 (없으면 nullptr)
    [[nodiscard]] const combat::Spell* spell() const { return spell_ ? &*spell_ : nullptr; }
    // 한 번 쓸 때 MP (마술이 없으면 0)
    [[nodiscard]] float mp_cost() const;

private:
    std::optional<combat::Spell> spell_;
    bool casting_ = false;
    float elapsed_ = 0.0f;  // 시전 시작 뒤 흐른 시간
};

}  // namespace wind::game
