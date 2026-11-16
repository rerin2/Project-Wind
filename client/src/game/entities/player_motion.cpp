// 이 파일은 무엇을 하나요?
// 주인공 한 박자: 구르기·공격·달리기를 스태미나 규칙과 상태 전이 표에 맞게 허락하고,
// 쓴 만큼 스태미나를 줄여요.

#include "game/entities/player_motion.hpp"

#include "game/combat/hitbox_placement.hpp"
#include "game/entities/facing.hpp"
#include "game/input/action.hpp"

#include <utility>

namespace wind::game {
namespace {

// 시전 중 걷기 빠르기 배수 (걷기 110 px/s × 0.4 = 44 px/s). 이유는 docs/GAMEPLAY.md "Casting" 절.
constexpr float kCastWalkScale = 0.4f;

}  // namespace

PlayerMotion::PlayerMotion(engine::Vec2 feet, world::Direction facing, MovementTuning movement,
                           StaminaTuning stamina, double buffer_seconds, combat::Weapon weapon,
                           ManaTuning mana)
    : mover_(feet, facing, movement),
      stamina_(stamina),
      mana_(mana),
      attack_(std::move(weapon)),
      buffer_(buffer_seconds) {}

void PlayerMotion::press_dodge() {
    buffer_.press(Action::Dodge);
}

void PlayerMotion::press_attack() {
    buffer_.press(Action::Attack);
}

void PlayerMotion::press_cast() {
    buffer_.press(Action::Cast);
}

CharacterState PlayerMotion::state() const {
    if (attack_.busy()) {
        return CharacterState::Attacking;
    }
    if (caster_.busy()) {
        return CharacterState::Casting;
    }
    switch (mover_.state()) {
        case Mover::State::Idle: return CharacterState::Idle;
        case Mover::State::Walking: return CharacterState::Moving;
        case Mover::State::Running: return CharacterState::Running;
        case Mover::State::Dodging: return CharacterState::Dodging;
    }
    return CharacterState::Idle;
}

// 구르기: 표가 허락하고(공격 중이면 취소 구간에서만), 스태미나가 충분할 때만 메모를 꺼내요.
// 모자라면 메모는 남아 있다가(0.15초 안에 조건이 맞으면 그때 굴러요) 사라져요.
bool PlayerMotion::try_dodge() {
    const bool attack_allows = !attack_.busy() || attack_.in_cancel_window();
    // 시전 중에는 못 굴러요 (명세 8.5: 시전 시작 전 또는 광선 유지 중에만. 광선은 15단계)
    const bool cast_allows = !caster_.busy();
    const bool ok = can_transition(state(), CharacterState::Dodging) && attack_allows && cast_allows &&
                    mover_.can_dodge() && stamina_.can_afford(stamina_.tuning().dodge_cost);
    if (!ok || !buffer_.consume(Action::Dodge)) {
        return false;
    }
    attack_.cancel();  // 공격 중이었으면 후딜을 끊고 굴러요
    return true;
}

// 공격: 표가 허락하고, 휘두를 수 있고(처음이거나 취소 구간의 다음 콤보), 지침이 아니면 시작해요.
void PlayerMotion::try_attack(engine::Vec2 move) {
    const bool ok = can_transition(state(), CharacterState::Attacking) && attack_.can_start() &&
                    stamina_.can_attack() && buffer_.pending(Action::Attack);
    if (!ok) {
        return;
    }
    (void)buffer_.consume(Action::Attack);
    attack_.start();
    // 공격 방향은 누른 방향, 아무것도 안 누르면 바라보는 방향 (명세 9.2)
    mover_.face(facing_from_vector(move, mover_.facing()));
    // 모자라도 나가고 0이 돼요 (spend가 0 아래로는 안 내려가요).
    stamina_.spend(attack_.weapon().stamina_per_attack);
    attack_started_ = true;
}

// 마술: 표가 허락하고 시전할 수 있을 때 메모를 꺼내요.
// MP는 "전부 아니면 전부 안 함"이라, 모자라면 하나도 안 줄고 마술도 안 나가요 (명세 9.2).
void PlayerMotion::try_cast(engine::Vec2 move) {
    if (!buffer_.pending(Action::Cast)) {
        return;  // 누르지 않았어요
    }
    if (!can_transition(state(), CharacterState::Casting) || !caster_.can_start()) {
        return;  // 지금은 못 써요. 메모는 0.15초 동안 남아 있다가 사라져요
    }
    (void)buffer_.consume(Action::Cast);  // 이번 누름은 여기서 처리 끝
    if (!mana_.try_spend(caster_.mp_cost())) {
        ++mana_denied_count_;  // MP 모자람: HUD MP 바가 깜빡여요
        return;
    }
    caster_.start();
    // 마술 방향은 누른 방향, 아무것도 안 누르면 바라보는 방향. 시전하는 동안 고정돼요.
    cast_facing_ = facing_from_vector(move, mover_.facing());
    mover_.face(cast_facing_);
}

bool PlayerMotion::step(double dt_seconds, engine::Vec2 move, bool run_held,
                        const world::MapData& map) {
    const auto dt = static_cast<float>(dt_seconds);
    attack_started_ = false;
    spell_released_ = false;

    // 1) 구르기 먼저: 버튼을 같이 눌렀으면 피하는 쪽이 이겨요.
    const bool dodge = try_dodge();
    // 2) 공격, 3) 마술 (구르기 중이면 표가 막아요. 공격 중에는 마술을, 시전 중에는 공격을 표가 막아요)
    if (!dodge) {
        try_attack(move);
        try_cast(move);
    }
    // 4) 공격 중에는 다리가 멈추고(서서히), 시전 중에는 천천히 걸어요. 둘 다 달리지 않아요.
    const bool attacking = attack_.busy();
    const bool casting = caster_.busy();
    const engine::Vec2 legs = attacking ? engine::Vec2{} : (casting ? move * kCastWalkScale : move);
    // 5) 달리기: Shift + 방향키 + 지침 아님
    const bool run = !attacking && !casting && run_held && engine::length(move) > 0.0f &&
                     stamina_.can_run();

    const bool started = mover_.step(dt_seconds, legs, dodge, map, run);
    if (started) {
        stamina_.spend(stamina_.tuning().dodge_cost);
    }
    if (mover_.state() == Mover::State::Running) {
        stamina_.spend(stamina_.tuning().run_cost_per_second * dt);
    }
    if (casting) {
        mover_.face(cast_facing_);  // 시전 중에는 걸어도 마술 쪽을 계속 봐요
    }
    attack_.step(dt);
    // 6) 스태미나: 이번 박자에 썼으면 기다리는 시간이 다시 시작돼서, 이 update는 채우지 않아요.
    stamina_.update(dt);
    //    MP는 기다림 없이 늘 조금씩 차요 (1초에 2).
    mana_.update(dt);
    // 7) 시전 시간이 다 차면 이번 박자에 마술이 나가요
    spell_released_ = caster_.step(dt);
    buffer_.tick(dt_seconds);
    return started;
}

std::optional<engine::RectF> PlayerMotion::attack_hitbox() const {
    if (attack_.phase() != AttackRunner::Phase::Active) {
        return std::nullopt;
    }
    return place_hitbox(attack_.current()->hitbox, mover_.feet(), mover_.facing());
}

}  // namespace wind::game
