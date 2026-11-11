#pragma once

// 이 파일은 무엇을 하나요?
// AttackRunner = 무기 휘두르기를 "시간에 맞춰 진행하는" 담당이에요. (명세 9.2)
// - 공격 하나는 준비(windup) -> 판정(active) -> 후딜(recovery) 순서로 흘러가요.
// - 취소 구간(cancel window) 안에서 다시 공격하면 다음 콤보(2타, 3타)로 이어져요.
// - 판정 구간에 히트박스에 닿은 대상은 "한 번 휘두를 때 한 번만" 맞아요.
//   (같은 허수아비가 한 휘두르기에 두 번 깎이면 안 되니까요)
// 스태미나와 입력 버퍼는 PlayerMotion이 맡고, 이 담당은 시간과 콤보 순서만 봐요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "wind/combat/attack.hpp"
#include "wind/combat/weapon.hpp"

#include <cstdint>
#include <vector>

namespace wind::game {

// 맞을 수 있는 대상의 번호 (허수아비, 몬스터 ...)
using TargetId = std::uint32_t;

class AttackRunner {
public:
    enum class Phase : std::uint8_t { None, Windup, Active, Recovery };

    // 무기가 없으면(콤보가 비었으면) 공격을 시작할 수 없어요.
    explicit AttackRunner(combat::Weapon weapon = {});

    // 지금 새 공격을 시작할 수 있나요?
    // - 휘두르는 중이 아니면: 1타부터 시작
    // - 휘두르는 중이면: 취소 구간 안이고 다음 콤보가 있을 때만
    [[nodiscard]] bool can_start() const;
    // 공격 시작 (can_start()가 false면 아무 일도 없고 false)
    bool start();
    // 공격을 바로 끝내요 (구르기로 취소할 때). 콤보도 처음으로 돌아가요.
    void cancel();
    // 한 박자 흘려보내요. 공격이 끝나면 쉬는 상태(Phase::None)가 돼요.
    void step(float dt_seconds);

    // 이번 휘두르기에서 이 대상을 처음 맞혔으면 true (그리고 기억해 둬요).
    // 판정 구간이 아니거나 이미 맞힌 대상이면 false.
    bool register_hit(TargetId target);

    [[nodiscard]] bool has_weapon() const { return !weapon_.combo.empty(); }
    [[nodiscard]] bool busy() const { return phase() != Phase::None; }
    [[nodiscard]] Phase phase() const;
    [[nodiscard]] bool in_cancel_window() const;
    // 지금 휘두르는 공격 (없으면 nullptr)
    [[nodiscard]] const combat::AttackData* current() const;
    // 콤보 몇 번째인지 (0 = 1타). 쉬는 중이면 -1
    [[nodiscard]] int combo_index() const { return index_; }
    // 지금 공격을 시작한 뒤 흐른 시간
    [[nodiscard]] float elapsed() const { return elapsed_; }
    [[nodiscard]] const combat::Weapon& weapon() const { return weapon_; }

private:
    combat::Weapon weapon_;
    int index_ = -1;        // 지금 콤보 번호 (-1 = 쉬는 중)
    float elapsed_ = 0.0f;  // 지금 공격 시작 뒤 흐른 시간
    std::vector<TargetId> hit_targets_;  // 이번 휘두르기에서 이미 맞힌 대상들
};

}  // namespace wind::game
