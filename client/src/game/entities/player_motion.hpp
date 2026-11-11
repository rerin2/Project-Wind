#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 몸 움직임 규칙을 한데 묶어요. (명세 7.2, 7.2.1, 8.5, 9.2)
// - Mover(다리): 걷기, 달리기, 구르기, 벽 미끄러짐
// - Stamina(기력): 달리기·구르기·공격에 쓰고, 쉬면 다시 참
// - AttackRunner(칼 휘두르기): 준비 -> 판정 -> 후딜, 콤보
// - InputBuffer(버튼 메모장): 구르기·공격 버튼을 0.15초 기억
// 한 박자 순서:
//   1) 구르기 메모가 있고, 구를 수 있으면 -> 구르기 (스태미나 25 씀)
//      공격 중이면 취소 구간에서만 굴러요 (공격은 취소돼요)
//   2) 공격 메모가 있고, 휘두를 수 있고, 지침이 아니고 스태미나가 0보다 많으면
//      -> 공격 시작 또는 다음 콤보 (무기의 공격 스태미나만큼 씀, 모자라면 0이 돼요)
//   3) 공격 중에는 발이 멈추고(서서히 감속), 달리지 않아요
//   4) Shift를 누르고 움직이며 지침이 아니면 -> 달리기 (달리는 동안 1초에 25 씀)
//   5) 쓰지 않았으면 0.6초 뒤부터 다시 참
// 어떤 상태에서 무엇을 시작할 수 있는지는 상태 전이 표(character_state)를 따라요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/attack_runner.hpp"
#include "game/entities/character_state.hpp"
#include "game/entities/mover.hpp"
#include "game/entities/stamina.hpp"
#include "game/input/input_buffer.hpp"

#include "wind/combat/weapon.hpp"
#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

#include <optional>

namespace wind::game {

class PlayerMotion {
public:
    // buffer_seconds: 구르기·공격 버튼을 몇 초 동안 기억할지
    // weapon: 든 무기 (콤보가 비어 있으면 공격을 못 해요)
    PlayerMotion(engine::Vec2 feet, world::Direction facing, MovementTuning movement,
                 StaminaTuning stamina, double buffer_seconds, combat::Weapon weapon = {});

    // 구르기 버튼을 눌렀어요 (화면 한 장마다, 박자 계산 전에). 메모장에 적어 둬요.
    void press_dodge();
    // 공격 버튼을 눌렀어요. 메모장에 적어 둬요.
    void press_attack();
    // 한 박자. move: 가고 싶은 방향, run_held: Shift를 누르고 있나
    // 구르기를 시작했으면 true
    bool step(double dt_seconds, engine::Vec2 move, bool run_held, const world::MapData& map);

    // 지금 판정 구간이면 공격이 닿는 네모 (맵 위 픽셀), 아니면 nullopt
    [[nodiscard]] std::optional<engine::RectF> attack_hitbox() const;
    // 이번 휘두르기에서 이 대상을 처음 맞혔으면 true (판정 구간에서만)
    bool register_hit(TargetId target) { return attack_.register_hit(target); }

    // 지금 상태 (idle, moving, running, attacking, dodging)
    [[nodiscard]] CharacterState state() const;
    // 이번 박자에 공격(또는 다음 콤보)을 시작했나 (연출용)
    [[nodiscard]] bool attack_started() const { return attack_started_; }

    [[nodiscard]] const Mover& mover() const { return mover_; }
    [[nodiscard]] const Stamina& stamina() const { return stamina_; }
    [[nodiscard]] const AttackRunner& attack() const { return attack_; }

private:
    bool try_dodge();
    void try_attack(engine::Vec2 move);

    Mover mover_;
    Stamina stamina_;
    AttackRunner attack_;
    InputBuffer buffer_;
    bool attack_started_ = false;
};

}  // namespace wind::game
