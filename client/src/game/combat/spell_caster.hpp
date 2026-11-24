#pragma once

// 이 파일은 무엇을 하나요?
// SpellCaster = 마술 "시전 시간"을 재는 담당이에요. (명세 8.6)
// - 시작하면 cast_seconds(별빛 조각 0.25초) 동안 모아요.
// - 빛 탄·근접 마술: 다 모은 박자에 "나갔다!"(step이 true)를 알려 줘요. 그때 빛 탄이 생겨요.
// - 광선 마술(혜성 광선, 15d): 다 모으면 "광선 유지" 단계로 넘어가요. 버튼을 누르고 있는 동안 계속돼요.
//   모으는 중이나 유지 중에 버튼을 떼면 바로 끝나요 (모으는 중에 떼면 광선은 안 나가요).
// MP와 입력 버퍼, 방향은 PlayerMotion이 맡고, 이 담당은 시간과 "누르고 있나"만 봐요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "wind/combat/spell.hpp"

#include <optional>

namespace wind::game {

class SpellCaster {
public:
    explicit SpellCaster(std::optional<combat::Spell> spell = {});

    // 고른 마술을 바꿔요 (없으면 마술을 못 써요). 시전 중이면 취소돼요.
    void set_spell(std::optional<combat::Spell> spell);

    // 지금 시전을 시작할 수 있나요? (마술이 있고, 움직일 수 있는 마술이고, 시전 중이 아님)
    // 못 쓰는 마술이면 PlayerMotion이 MP를 쓰지 않아요.
    [[nodiscard]] bool can_start() const;
    // 시전 시작 (can_start()가 false면 아무 일도 없고 false)
    bool start();
    // 시전·광선을 바로 멈춰요 (맞았을 때, 구를 때, MP가 떨어졌을 때). 마술은 나가지 않아요.
    void cancel();
    // 한 박자 흘려보내요. held: 마술 버튼을 누르고 있나 (광선만 봐요).
    // 이번 박자에 빛 탄·근접 마술이 나갔으면 true (광선은 늘 false, 대신 beaming()이 켜져요).
    bool step(float dt_seconds, bool held = true);

    // 모으는 중이거나 광선을 쏘는 중
    [[nodiscard]] bool busy() const { return phase_ != Phase::Idle; }
    // 광선을 쏘는 중 (15d)
    [[nodiscard]] bool beaming() const { return phase_ == Phase::Beaming; }
    // 모은 정도 0~1 (연출용: 손끝 빛이 커져요). 모으는 중이 아니면 0
    [[nodiscard]] float progress() const;
    // 고른 마술 (없으면 nullptr)
    [[nodiscard]] const combat::Spell* spell() const { return spell_ ? &*spell_ : nullptr; }
    // 시작할 때 내는 MP (마술이 없거나 광선이면 0: 광선은 쏘는 동안 1초마다 내요)
    [[nodiscard]] float mp_cost() const;
    // 시작하려면 있어야 하는 MP (combat::start_mp, 마술이 없으면 0)
    [[nodiscard]] float mp_to_start() const;

private:
    enum class Phase : unsigned char { Idle, Charging, Beaming };

    std::optional<combat::Spell> spell_;
    Phase phase_ = Phase::Idle;
    float elapsed_ = 0.0f;  // 모으기 시작한 뒤 흐른 시간
};

}  // namespace wind::game
