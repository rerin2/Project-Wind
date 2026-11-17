#pragma once

// 이 파일은 무엇을 하나요?
// 맵 위의 몬스터 한 마리예요. (명세 9.6, 11a·11b)
// - 처음 놓인 곳이 "제자리"(home)예요.
// - 대기 → 배회를 되풀이하다가, 주인공이 감지 거리 안에 오면 알아채요(느낌표).
// - 쫓기 → 공격 거리 안이면 공격 예고(멈춰서 웅크림) → 예고 때 정한 방향으로 덮치기 → 후딜.
// - 제자리에서 너무 멀어지거나 주인공이 너무 멀리 도망가면 제자리로 돌아가요.
//   돌아가는 중에도 주인공이 슬라임 영역 안에 다시 들어오면 바로 알아채요.
// - 걷기는 벽에 닿으면 벽을 따라 미끄러져요 (monster_moves).
// - 주사위는 FieldCombat의 시드 있는 주사위를 빌려 써요. 같은 씨앗·같은 주인공 움직임이면 늘 같은 결과예요.
// 게임 규칙이라 고정 박자로 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.
// 덮치기 판정은 FieldCombat이 주인공 맞는 상자와 비교해요 (11c). 몬스터가 맞고 쓰러지는 것은 11d예요.

#include "engine/core/math.hpp"
#include "game/monsters/monster_state.hpp"
#include "game/physics/tile_collision.hpp"

#include "wind/combat/monster_data.hpp"
#include "wind/world/map.hpp"

#include <optional>
#include <string>

namespace wind::game {

class CombatRandom;

class Monster {
public:
    // name: 맵 안 이름(로그용), data: 몬스터 설명 (공격이 하나 이상 있어야 덤벼요), home: 제자리 발 위치 (픽셀)
    Monster(std::string name, combat::MonsterData data, engine::Vec2 home);

    // 한 박자 움직여요. hero: 주인공 발 위치 (없으면 알아챌 대상이 없어요).
    // dt가 0이면(히트스톱) 아무것도 바뀌지 않아요.
    void step(float dt_seconds, const world::MapData& map, CombatRandom& random,
              std::optional<engine::Vec2> hero = std::nullopt);

    [[nodiscard]] MonsterState state() const { return state_; }
    // 지금 상태가 된 뒤 흐른 시간 (초)
    [[nodiscard]] float state_seconds() const { return state_seconds_; }
    // 시간이 정해진 상태(알아채기·예고·공격·후딜)의 진행 0~1. 다른 상태는 0이에요.
    [[nodiscard]] float state_progress() const;
    [[nodiscard]] engine::Vec2 feet() const { return feet_; }
    // 화면에 그릴 위치: 전 박자 위치와 지금 위치 사이 (alpha = 0이면 전, 1이면 지금)
    [[nodiscard]] engine::Vec2 interpolated_feet(float alpha) const;
    [[nodiscard]] engine::Vec2 home() const { return home_; }
    // 지난 박자에 움직인 빠르기와 방향 (픽셀/초)
    [[nodiscard]] engine::Vec2 velocity() const { return velocity_; }
    // 맵에 나온 뒤 걸은 거리의 합 (픽셀). 그림이 이걸 보고 통통 튀는 박자를 맞춰요.
    [[nodiscard]] float distance_moved() const { return distance_moved_; }
    // 맵 위의 맞는 상자 (픽셀)
    [[nodiscard]] engine::RectF hurtbox() const;
    // 벽에 부딪히는 발 상자 크기
    [[nodiscard]] BodyShape body() const { return {data_.body_width, data_.body_height}; }
    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] const combat::MonsterData& data() const { return data_; }

    // 지금(또는 다음에) 쓰는 공격. 공격이 없는 몬스터면 nullptr
    [[nodiscard]] const combat::AttackData* attack() const;
    // 공격 예고를 시작할 때 정한 덮칠 방향 (길이 1). 예고·공격·후딜 동안 바뀌지 않아요.
    [[nodiscard]] engine::Vec2 attack_direction() const { return attack_direction_; }
    // 공격 예고 동안 바닥에 보일 "덮칠 길": 지금 발에서 덮치기 끝까지 (예고가 아니면 없음)
    [[nodiscard]] std::optional<engine::Vec2> lunge_end() const;
    // 공격 판정 상자 (덮치는 동안만 있어요, 맵 위 픽셀)
    [[nodiscard]] std::optional<engine::RectF> attack_hitbox() const;
    // 덮치기 한 번에 한 번만 맞히게: 이번 덮치기에서 처음이면 true를 돌려주고 기억해요 (11c)
    bool claim_hit();
    // 밖에서 밀어요 (11c: 몸끼리 밀어내기). 벽에 닿으면 벽을 따라 미끄러져요. 상태는 바꾸지 않아요.
    void shove(engine::Vec2 delta, const world::MapData& map);

private:
    // 상태를 바꾸고 상태 시계를 0으로
    void enter(MonsterState state);
    // 쉬기 시작: 쉬는 시간을 주사위로 골라요
    void start_idle(CombatRandom& random);
    // 걷기 시작: 제자리 둘레에서 걸어갈 점을 골라요. 못 고르면 다시 쉬어요.
    void start_wander(const world::MapData& map, CombatRandom& random);
    // 공격 예고 시작: 주인공 쪽으로 덮칠 방향을 정해요
    void start_windup(engine::Vec2 hero);
    // 한 박자 걷기 결과를 몸에 반영해요 (위치, 빠르기, 걸은 거리)
    void apply_move(engine::Vec2 new_feet, float travelled, float dt_seconds);

    // 상태마다 한 박자
    void step_rest(float dt_seconds, const world::MapData& map, CombatRandom& random);
    void step_wander(float dt_seconds, const world::MapData& map, CombatRandom& random);
    void step_chase(float dt_seconds, const world::MapData& map, std::optional<engine::Vec2> hero);
    void step_attack(float dt_seconds, const world::MapData& map);
    void step_return(float dt_seconds, const world::MapData& map, CombatRandom& random,
                     std::optional<engine::Vec2> hero);
    // 후딜이 끝났을 때: 계속 쫓을지, 돌아갈지
    void after_recover(std::optional<engine::Vec2> hero);

    // 쉬거나 걷다가 주인공을 알아채나요? (감지 거리 안)
    [[nodiscard]] bool notices(std::optional<engine::Vec2> hero) const;
    // 돌아가다가 다시 알아채나요? (감지 거리 안 + 주인공이 슬라임 영역 안)
    [[nodiscard]] bool notices_while_returning(std::optional<engine::Vec2> hero) const;
    // 계속 쫓아도 되나요? (제자리에서도, 주인공에게서도 추적 한계 안)
    [[nodiscard]] bool can_keep_chasing(std::optional<engine::Vec2> hero) const;

    std::string name_;              // 맵 안 이름 ("meadow_slime_1")
    combat::MonsterData data_;      // HP·빠르기·거리·그림·공격 설명
    engine::Vec2 home_;             // 제자리 (처음 놓인 발 위치)
    engine::Vec2 feet_;             // 지금 발 위치
    engine::Vec2 previous_feet_;    // 전 박자 발 위치 (부드럽게 그리기용)
    engine::Vec2 velocity_;         // 지난 박자 빠르기
    engine::Vec2 target_;           // 배회 중 걸어가는 점
    engine::Vec2 attack_direction_{0.0f, 1.0f};  // 덮칠 방향 (예고 때 정해요)
    MonsterState state_ = MonsterState::Idle;
    float state_seconds_ = 0.0f;    // 지금 상태가 된 뒤 흐른 시간 (초)
    float idle_left_ = 0.0f;        // 쉬는 시간 남은 것 (초)
    float stuck_seconds_ = 0.0f;    // 거의 못 가고 있는 시간 (초, 배회·복귀)
    float distance_moved_ = 0.0f;   // 걸은 거리의 합 (픽셀)
    bool hit_landed_ = false;       // 이번 덮치기에서 이미 맞혔나 (예고를 시작할 때 지워요)
};

}  // namespace wind::game
