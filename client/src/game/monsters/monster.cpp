// 이 파일은 무엇을 하나요?
// 몬스터 한 마리의 상태(대기·배회·알아채기·쫓기·예고·공격·후딜·복귀)를 박자마다 바꾸는 곳이에요. (11a, 11b)
// 규칙과 값의 이유는 docs/GAMEPLAY.md "Monsters" 절에 있어요.

#include "game/monsters/monster.hpp"

#include "game/combat/combat_random.hpp"
#include "game/combat/hitbox_placement.hpp"
#include "game/entities/facing.hpp"
#include "game/monsters/monster_moves.hpp"
#include "game/monsters/monster_senses.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {
namespace {

// 걸어갈 점을 고를 때 몇 번까지 다시 굴려 볼지. 둥근 범위 밖이거나 벽 속 점이면 다시 굴려요.
constexpr int kTargetTries = 8;
// 남은 거리가 이보다 짧은 점은 "이미 도착"이라 걷지 않아요 (픽셀)
constexpr float kArriveDistance = 1.0f;
// 배회 중 막힌 채로 이만큼 지나면 포기하고 쉬어요 (초). 벽을 따라 미끄러질 시간은 줘요.
constexpr float kWanderGiveUpSeconds = 0.5f;
// 돌아가는 중 막힌 채로 이만큼 지나면 그 자리에서 쉬어요 (초). 나무 뒤에 영원히 걸리지 않게.
constexpr float kReturnGiveUpSeconds = 1.0f;
// 쓰러진 뒤 사라지기까지 (초, 11d). 이 동안 깜빡이며 납작해지는 연출이 나와요.
constexpr float kDeathSeconds = 0.5f;

// 두 점 사이 거리
float distance(engine::Vec2 a, engine::Vec2 b) { return engine::length(a - b); }

}  // namespace

Monster::Monster(std::string name, combat::MonsterData data, engine::Vec2 home)
    : name_(std::move(name)),
      data_(std::move(data)),
      home_(home),
      feet_(home),
      previous_feet_(home),
      target_(home),
      idle_left_(data_.wander.pause_min),  // 처음 쉬는 시간은 최소값 (주사위는 박자 안에서만 굴려요)
      hp_(data_.max_hp),                   // 가득 찬 HP로 나타나요
      hit_(HitTuning{data_.stagger_seconds, 0.0f, {}}) {}  // 움찔 시간은 데이터, 무적은 없음

void Monster::step(float dt_seconds, const world::MapData& map, CombatRandom& random,
                   std::optional<engine::Vec2> hero) {
    previous_feet_ = feet_;                // 부드럽게 그리기: 이번 박자 시작 위치
    velocity_ = {};                        // 걷지 않으면 빠르기 0
    if (!(dt_seconds > 0.0f)) return;      // 히트스톱(0)이나 이상한 값이면 그대로
    state_seconds_ += dt_seconds;          // 상태 시계
    shove(hit_.step(dt_seconds), map);     // 맞았으면 밀려나요 (11d, 벽 미끄러짐). 덮치는 중·쓰러진 뒤에도요
    const combat::AttackData* move = attack();
    switch (state_) {
        case MonsterState::Idle:
        case MonsterState::Wander:
            if (monster_notices(data_, feet_, hero)) {  // 쉬거나 걷다가 주인공을 봤어요!
                enter(MonsterState::Notice);
                return;
            }
            if (state_ == MonsterState::Idle) {
                step_rest(dt_seconds, map, random);
            } else {
                step_wander(dt_seconds, map, random);
            }
            break;
        case MonsterState::Notice:         // 느낌표를 띄우고 멈춰 있다가 쫓기 시작
            if (state_seconds_ >= data_.notice_seconds) enter(MonsterState::Chase);
            break;
        case MonsterState::Chase:
            step_chase(dt_seconds, map, hero);
            break;
        case MonsterState::Windup:         // 멈춰서 웅크리고 있다가 덮치기
            if (move != nullptr && state_seconds_ >= move->windup) enter(MonsterState::Attack);
            break;
        case MonsterState::Attack:
            step_attack(dt_seconds, map);
            break;
        case MonsterState::Recover:        // 빈틈이 끝나면 계속 쫓을지 돌아갈지 정해요
            if (move == nullptr || state_seconds_ >= move->recovery) after_recover(hero);
            break;
        case MonsterState::Hurt:           // 움찔이 끝나면 쫓거나(맞으면 알아채요) 돌아가요
            if (!hit_.staggered()) after_recover(hero);
            break;
        case MonsterState::Dead:           // 쓰러졌으면 아무것도 안 해요 (사라지는 건 gone())
            break;
        case MonsterState::Return:
            step_return(dt_seconds, map, random, hero);
            break;
    }
}

float Monster::state_progress() const {
    const combat::AttackData* move = attack();
    float total = 0.0f;  // 이 상태가 얼마나 이어지는지 (초)
    switch (state_) {
        case MonsterState::Notice: total = data_.notice_seconds; break;
        case MonsterState::Windup: total = move != nullptr ? move->windup : 0.0f; break;
        case MonsterState::Attack: total = move != nullptr ? move->active : 0.0f; break;
        case MonsterState::Recover: total = move != nullptr ? move->recovery : 0.0f; break;
        case MonsterState::Hurt: total = data_.stagger_seconds; break;
        case MonsterState::Dead: total = kDeathSeconds; break;
        default: return 0.0f;  // 시간이 정해지지 않은 상태
    }
    return total > 0.0f ? std::clamp(state_seconds_ / total, 0.0f, 1.0f) : 1.0f;
}

engine::Vec2 Monster::interpolated_feet(float alpha) const {
    return engine::lerp(previous_feet_, feet_, std::clamp(alpha, 0.0f, 1.0f));
}

engine::RectF Monster::hurtbox() const {
    return {feet_.x + data_.hurtbox.x, feet_.y + data_.hurtbox.y, data_.hurtbox.w, data_.hurtbox.h};
}

const combat::AttackData* Monster::attack() const {
    return data_.attacks.empty() ? nullptr : &data_.attacks.front();  // 11b: 첫 번째 공격만 써요
}

std::optional<engine::Vec2> Monster::lunge_end() const {
    const combat::AttackData* move = attack();
    if (state_ != MonsterState::Windup || move == nullptr) return std::nullopt;
    // 지금 자리에서 덮치기 거리만큼 앞. 침(13c)이면 침이 날아갈 거리 끝까지 보여 줘요.
    const float reach = move->projectile ? move->projectile->range : move->lunge;
    return feet_ + attack_direction_ * reach;
}

std::optional<engine::RectF> Monster::attack_hitbox() const {
    const combat::AttackData* move = attack();
    // 침을 쏘는 공격(13c)은 몸으로 부딪히지 않아요: 판정은 날아가는 침이 해요 (FieldCombat)
    if (state_ != MonsterState::Attack || move == nullptr || move->projectile) return std::nullopt;
    // 히트박스는 4방향 기준이라, 덮칠 방향에 가장 가까운 방향으로 놓아요
    const world::Direction facing = facing_from_vector(attack_direction_, world::Direction::Down);
    return place_hitbox(move->hitbox, feet_, facing);
}

int Monster::take_damage(int amount) {
    if (!can_be_hit() || amount <= 0) return 0;     // 쓰러졌거나 0·음수 피해면 그대로
    const int applied = std::min(hp_, amount);       // HP보다 많이는 못 깎아요
    hp_ -= applied;
    if (hp_ <= 0) enter(MonsterState::Dead);         // HP 0: 쓰러져요 (공격도 예고도 끝)
    return applied;
}

void Monster::push(engine::Vec2 direction, float knockback) {
    hit_.start(direction, knockback);                // 밀리기 시작 (다음 박자부터, 벽 미끄러짐)
    // 덮치는 중이면 덮치기를 계속해요 (개발자와 정함). 쓰러졌으면 그대로 쓰러져 있어요.
    if (state_ == MonsterState::Attack || state_ == MonsterState::Dead) return;
    enter(MonsterState::Hurt);                       // 그 밖에는 움찔: 예고 중이었으면 공격이 취소돼요
}

bool Monster::gone() const {
    return state_ == MonsterState::Dead && state_seconds_ >= kDeathSeconds;
}

bool Monster::claim_hit() {
    if (state_ != MonsterState::Attack || hit_landed_) return false;  // 덮치는 중이 아니거나 이미 맞혔으면
    hit_landed_ = true;
    return true;
}

void Monster::shove(engine::Vec2 delta, const world::MapData& map) {
    if (!(engine::length(delta) > 0.0f)) return;  // 0이나 이상한 값이면 그대로
    feet_ = move_and_slide(map, body(), feet_, delta, 0.0f).feet;  // 벽을 따라 미끄러지며 밀려요
}

void Monster::enter(MonsterState state) {
    state_ = state;
    state_seconds_ = 0.0f;
    stuck_seconds_ = 0.0f;
    // 날아가는 공격(침)이면 공격을 시작하는 순간 한 방을 쏠 준비 (13c). 그 밖에는 쏠 것이 없어요.
    shot_ready_ = state == MonsterState::Attack && attack() != nullptr && attack()->projectile.has_value();
}

void Monster::start_idle(CombatRandom& random) {
    enter(MonsterState::Idle);
    const auto& wander = data_.wander;
    // 쉬는 시간 = 최소 + (최대 - 최소) × 주사위(0~1)
    idle_left_ = wander.pause_min + (wander.pause_max - wander.pause_min) * random.next_unit();
}

void Monster::start_wander(const world::MapData& map, CombatRandom& random) {
    const float radius = data_.wander.radius;
    for (int i = 0; i < kTargetTries; ++i) {
        // 네모 안의 점을 굴리고, 둥근 범위 밖이면 버려요. (sin·cos 없이 어느 컴퓨터에서나 같은 값)
        const engine::Vec2 offset{(random.next_unit() * 2.0f - 1.0f) * radius,
                                  (random.next_unit() * 2.0f - 1.0f) * radius};
        if (offset.x * offset.x + offset.y * offset.y > radius * radius) continue;  // 동그라미 밖
        const engine::Vec2 target = home_ + offset;                                // 제자리 기준 점
        if (box_blocked(map, body_box(target, body()))) continue;                  // 벽 속이면 다시
        if (distance(target, feet_) < kArriveDistance) continue;                   // 이미 거기 있음
        target_ = target;                       // 좋은 점을 골랐어요
        enter(MonsterState::Wander);
        return;
    }
    start_idle(random);  // 좋은 점이 없으면 조금 더 쉬어요
}

void Monster::start_windup(engine::Vec2 hero) {
    const engine::Vec2 toward = engine::normalized(hero - feet_);  // 주인공 쪽
    if (engine::length(toward) > 0.0f) attack_direction_ = toward;  // 겹쳐 있으면 지난 방향 그대로
    hit_landed_ = false;                                            // 새 공격: 아직 아무도 안 맞혔어요
    enter(MonsterState::Windup);
}

void Monster::apply_move(engine::Vec2 new_feet, float travelled, float dt_seconds) {
    velocity_ = (new_feet - feet_) * (1.0f / dt_seconds);
    feet_ = new_feet;
    distance_moved_ += travelled;
}

void Monster::step_rest(float dt_seconds, const world::MapData& map, CombatRandom& random) {
    idle_left_ -= dt_seconds;                  // 쉬는 시간 줄이기
    if (idle_left_ <= 0.0f) start_wander(map, random);  // 다 쉬었으면 걸어갈 점 고르기
}

void Monster::step_wander(float dt_seconds, const world::MapData& map, CombatRandom& random) {
    const MonsterStep step = step_toward(map, body(), feet_, target_, data_.wander.speed, dt_seconds);
    apply_move(step.feet, step.travelled, dt_seconds);
    if (step.arrived) {                        // 막힘 없이 도착하면 쉬어요
        start_idle(random);
        return;
    }
    stuck_seconds_ = step.blocked ? stuck_seconds_ + dt_seconds : 0.0f;  // 막힌 시간 세기
    if (stuck_seconds_ >= kWanderGiveUpSeconds) start_idle(random);     // 오래 막히면 포기
}

void Monster::step_chase(float dt_seconds, const world::MapData& map, std::optional<engine::Vec2> hero) {
    if (!monster_can_keep_chasing(data_, feet_, home_, hero)) {  // 너무 멀어졌으면 포기하고 돌아가요
        enter(MonsterState::Return);
        return;
    }
    const float to_hero = distance(*hero, feet_);
    // 너무 가까우면 물러나요 (keep_away, 개구리 13c). 벽에 막혀 못 물러나면 그 자리에서 공격해요.
    if (to_hero < data_.keep_away && step_back(dt_seconds, map, *hero)) return;
    if (attack() != nullptr && to_hero <= data_.attack_range) {
        start_windup(*hero);                   // 공격 거리 안: 멈춰서 예고
        return;
    }
    // 주인공 쪽으로 걸어가요 (벽에 닿으면 미끄러져요. 길 찾기는 아직 없어요, 명세 9.6)
    const MonsterStep step = step_toward(map, body(), feet_, *hero, data_.move_speed, dt_seconds);
    apply_move(step.feet, step.travelled, dt_seconds);
}

void Monster::step_attack(float dt_seconds, const world::MapData& map) {
    const combat::AttackData* move = attack();
    if (move == nullptr) {                     // 공격이 없는 몬스터는 덮칠 수 없어요 (예고로 오지 않지만 안전하게)
        enter(MonsterState::Recover);
        return;
    }
    // 판정 구간 동안 덮치기 거리를 고르게 나눠서 나가요 (구간을 넘친 시간만큼은 덜 가요)
    const float overshoot = std::max(0.0f, state_seconds_ - move->active);
    const float moving = std::max(0.0f, dt_seconds - overshoot);
    const float speed = move->active > 0.0f ? move->lunge / move->active : 0.0f;
    if (moving > 0.0f && speed > 0.0f) {
        const MonsterStep step = step_along(map, body(), feet_, attack_direction_, speed * moving);
        apply_move(step.feet, step.travelled, dt_seconds);
    }
    if (state_seconds_ >= move->active) enter(MonsterState::Recover);  // 판정 끝: 빈틈
}

void Monster::step_return(float dt_seconds, const world::MapData& map, CombatRandom& random,
                          std::optional<engine::Vec2> hero) {
    if (monster_notices_while_returning(data_, feet_, home_, hero)) {  // 주인공이 영역 안으로 다시 왔어요
        enter(MonsterState::Notice);
        return;
    }
    const MonsterStep step = step_toward(map, body(), feet_, home_, data_.move_speed, dt_seconds);
    apply_move(step.feet, step.travelled, dt_seconds);
    if (step.arrived) {                        // 제자리 도착: 다시 쉬기부터
        start_idle(random);
        return;
    }
    stuck_seconds_ = step.blocked ? stuck_seconds_ + dt_seconds : 0.0f;
    if (stuck_seconds_ >= kReturnGiveUpSeconds) start_idle(random);  // 오래 막히면 그 자리에서 쉬어요
}

void Monster::after_recover(std::optional<engine::Vec2> hero) {
    enter(monster_can_keep_chasing(data_, feet_, home_, hero) ? MonsterState::Chase : MonsterState::Return);
}

bool Monster::take_shot() {
    return std::exchange(shot_ready_, false);  // 한 번만 (FieldCombat이 침 한 방을 만들어요)
}

// 너무 가까우면 주인공 반대쪽으로 한 걸음 물러나요 (개구리, 13c). 막혀서 못 물러나면 false
bool Monster::step_back(float dt_seconds, const world::MapData& map, engine::Vec2 hero) {
    const engine::Vec2 away = engine::normalized(feet_ - hero);
    if (!(engine::length(away) > 0.0f)) return false;  // 딱 겹쳐 있으면 방향이 없어요
    const MonsterStep step = step_along(map, body(), feet_, away, data_.move_speed * dt_seconds);
    apply_move(step.feet, step.travelled, dt_seconds);
    return !step.blocked;
}

}  // namespace wind::game
