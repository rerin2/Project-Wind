#pragma once

// 이 파일은 무엇을 하나요?
// 맵 위의 몬스터 한 마리예요. (명세 9.6, 11a)
// - 처음 놓인 곳이 "제자리"(home)예요.
// - 대기(Idle): 정해진 범위에서 주사위로 고른 시간만큼 쉬어요.
// - 배회(Wander): 제자리 둘레(반지름 안)의 한 점을 주사위로 골라 천천히 걸어가요.
//   벽에 닿으면 벽을 따라 미끄러지고(tile_collision), 한동안 거의 못 가면 포기하고 다시 쉬어요.
// - 주사위는 FieldCombat의 시드 있는 주사위를 빌려 써요. 같은 씨앗이면 늘 같은 길로 걸어요.
// 게임 규칙이라 고정 박자로 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.
// 발견·추적·공격(11b), 맞기·쓰러지기(11d)는 다음 작업에서 더해요.

#include "engine/core/math.hpp"
#include "game/monsters/monster_state.hpp"
#include "game/physics/tile_collision.hpp"

#include "wind/combat/monster_data.hpp"
#include "wind/world/map.hpp"

#include <string>

namespace wind::game {

class CombatRandom;

class Monster {
public:
    // name: 맵 안 이름(로그용), data: 몬스터 설명, home: 제자리 발 위치 (픽셀)
    Monster(std::string name, combat::MonsterData data, engine::Vec2 home);

    // 한 박자 움직여요. dt가 0이면(히트스톱) 아무것도 바뀌지 않아요.
    void step(float dt_seconds, const world::MapData& map, CombatRandom& random);

    [[nodiscard]] MonsterState state() const { return state_; }
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

private:
    // 쉬기 시작: 쉬는 시간을 주사위로 골라요
    void start_idle(CombatRandom& random);
    // 걷기 시작: 제자리 둘레에서 걸어갈 점을 골라요. 못 고르면 다시 쉬어요.
    void start_wander(const world::MapData& map, CombatRandom& random);
    // 고른 점으로 한 박자 걸어요 (벽 미끄러짐, 막히면 포기)
    void walk(float dt_seconds, const world::MapData& map, CombatRandom& random);

    std::string name_;              // 맵 안 이름 ("meadow_slime_1")
    combat::MonsterData data_;      // HP·빠르기·거리·그림 설명
    engine::Vec2 home_;             // 제자리 (처음 놓인 발 위치)
    engine::Vec2 feet_;             // 지금 발 위치
    engine::Vec2 previous_feet_;    // 전 박자 발 위치 (부드럽게 그리기용)
    engine::Vec2 velocity_;         // 지난 박자 빠르기
    engine::Vec2 target_;           // 배회 중 걸어가는 점
    MonsterState state_ = MonsterState::Idle;
    float idle_left_ = 0.0f;        // 쉬는 시간 남은 것 (초)
    float stuck_seconds_ = 0.0f;    // 거의 못 가고 있는 시간 (초)
    float distance_moved_ = 0.0f;   // 걸은 거리의 합 (픽셀)
};

}  // namespace wind::game
