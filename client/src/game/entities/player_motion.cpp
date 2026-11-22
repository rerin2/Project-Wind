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
                           ManaTuning mana, HealthTuning health, HitTuning hit)
    : mover_(feet, facing, movement),
      stamina_(stamina),
      mana_(mana),
      attack_(std::move(weapon)),
      buffer_(buffer_seconds),
      health_(health),
      hit_(hit) {}

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
    if (dead()) {
        return CharacterState::Dead;  // 쓰러짐이 무엇보다 먼저: 표가 모든 행동을 막아요 (12b)
    }
    if (rest_.active()) {
        return CharacterState::Resting;  // 쿠션에서 쉬는 중: 표가 모든 행동을 막아요 (13b)
    }
    if (hit_.staggered()) {
        return CharacterState::Hit;  // 경직이 가장 먼저: 표가 모든 행동을 막아요 (11c)
    }
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
    if (dead()) {  // 9) 쓰러졌으면 아무 행동도 안 해요 (12b)
        step_dead(dt_seconds, map);
        return false;
    }
    seconds_since_hit_ += dt;  // 마지막으로 맞은 뒤 지난 시간 (쉬기 조건, 13b)
    if (rest_.active()) {      // 10) 쿠션에서 쉬는 중이면 앉기·일어나기만 해요 (13b)
        step_resting(dt_seconds, move, map);
        return false;
    }

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
    const bool staggered = hit_.staggered();  // 경직 중이면 다리도 멈춰요 (11c)
    const engine::Vec2 legs =
        (attacking || staggered) ? engine::Vec2{} : (casting ? move * kCastWalkScale : move);
    // 5) 달리기: Shift + 방향키 + 지침 아님
    const bool run = !attacking && !casting && !staggered && run_held && engine::length(move) > 0.0f &&
                     stamina_.can_run();

    const bool started = mover_.step(dt_seconds, legs, dodge, map, run);
    // 넉백: 경직 동안 맞은 쪽으로 밀려나요. 벽에 닿으면 벽을 따라 미끄러져요 (11c). 무적 시간도 여기서 줄어요
    mover_.shove(hit_.step(dt), map);
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

// 쓰러진 박자: 버튼 메모는 흘려보내고(0.15초 뒤 사라져요), 다리는 멈춘 채,
// 마지막 한 방의 넉백만 벽을 따라 끝까지 밀려나요. 스태미나·MP는 차지 않아요.
void PlayerMotion::step_dead(double dt_seconds, const world::MapData& map) {
    (void)mover_.step(dt_seconds, {}, false, map, false);  // 다리: 가만히 (위치 기록도 이어져요)
    mover_.shove(hit_.step(static_cast<float>(dt_seconds)), map);  // 넉백은 끝까지
    buffer_.tick(dt_seconds);                               // 눌렀던 버튼은 그냥 잊혀요
}

// 쉬는 박자: 앉아 있을 때 방향키를 누르면 일어나요. 다리는 멈추고, 옮겨 가는 동안만 쿠션 쪽으로 미끄러져요.
void PlayerMotion::step_resting(double dt_seconds, engine::Vec2 move, const world::MapData& map) {
    const auto dt = static_cast<float>(dt_seconds);
    if (rest_.phase() == HeroRest::Phase::Sitting && engine::length(move) > 0.0f) {
        (void)rest_.stand_up();  // 방향키: 기지개 켜고 일어나기 (E는 FieldRest가 stand_up을 불러요)
    }
    (void)mover_.step(dt_seconds, {}, false, map, false);  // 다리: 가만히 (위치 기록은 이어져요)
    mover_.shove(rest_.step(dt, mover_.feet()), map);      // 쿠션 가운데로 사뿐히 (벽에 닿으면 미끄러져요)
    stamina_.update(dt);                                   // 이미 가득이지만 규칙은 같아요
    mana_.update(dt);
    buffer_.tick(dt_seconds);                              // 눌렀던 버튼은 그냥 잊혀요
}

bool PlayerMotion::begin_rest(engine::Vec2 seat) {
    if (dead() || !can_transition(state(), CharacterState::Resting)) return false;  // 걷기·서기 중에만
    mover_.halt();          // 걷던 힘을 끊고
    buffer_.clear();        // 미리 눌러 둔 버튼도 잊어요 (일어나자마자 칼이 나가지 않게)
    health_.refill();       // 모든 상태 회복: HP·MP·스태미나 가득, 지침도 풀려요 (명세 8.7 쉬기 2번)
    mana_.refill();
    stamina_.refill();
    mover_.face(world::Direction::Down);  // 쿠션 위에서는 화면 쪽을 봐요 (앉은 그림이 앞모습뿐이에요)
    rest_.begin(seat);
    return true;
}

void PlayerMotion::sit_on(engine::Vec2 seat) {
    mover_.teleport(seat);                // 쿠션 자리에 딱
    mover_.face(world::Direction::Down);
    rest_.sit_now(seat);
}

bool PlayerMotion::can_be_hit() const {
    // 쓰러져 있지 않고(12b), 쉬는 중도 아니고(13b), 피격 무적도, 구르기 무적도 아니면 (11c)
    return !dead() && !rest_.active() && !hit_.invulnerable() && !mover_.invulnerable();
}

int PlayerMotion::take_hit(int amount, engine::Vec2 direction, float knockback) {
    if (dead()) {
        return 0;       // 이미 쓰러졌으면 더 맞지 않아요 (12b)
    }
    attack_.cancel();   // 휘두르던 칼은 끊겨요 (상태 표: attacking -> hit)
    caster_.cancel();   // 모으던 마술도 끊겨요. 먼저 낸 MP는 돌아오지 않아요 (casting -> hit)
    mover_.halt();      // 걷던 힘·구르던 힘도 끊겨요
    seconds_since_hit_ = 0.0f;               // 방금 맞았어요: 3초 동안 쉴 수 없어요 (13b)
    const int taken = health_.take(amount);  // HP 깎기 (12b: 0까지 내려가면 쓰러져요)
    hit_.start(direction, knockback);         // 경직·무적·넉백 시작 (쓰러져도 넉백은 밀려나요)
    return taken;
}

HeroVitals PlayerMotion::vitals() const {
    return {health_.current(), mana_.current(), stamina_.current(), stamina_.exhausted()};
}

void PlayerMotion::restore(const HeroVitals& vitals) {
    health_.restore(vitals.hp);                            // HP (0~최대)
    mana_.restore(vitals.mana);                            // MP (0~최대)
    stamina_.restore(vitals.stamina, vitals.exhausted);    // 스태미나와 지침
}

engine::RectF PlayerMotion::hurtbox() const {
    const engine::RectF& shape = hit_.tuning().hurtbox;  // 발 기준 상자
    const engine::Vec2 feet = mover_.feet();
    return {feet.x + shape.x, feet.y + shape.y, shape.w, shape.h};
}

std::optional<engine::RectF> PlayerMotion::attack_hitbox() const {
    if (attack_.phase() != AttackRunner::Phase::Active) {
        return std::nullopt;
    }
    return place_hitbox(attack_.current()->hitbox, mover_.feet(), mover_.facing());
}

}  // namespace wind::game
