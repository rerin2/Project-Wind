#pragma once

// 이 파일은 무엇을 하나요?
// Mover = 캐릭터를 실제로 움직이는 "다리"예요. (명세 7.2, 8.5)
// - 가고 싶은 방향을 받아서 서서히 빨라지고(가속), 키를 떼면 서서히 멈춰요(감속).
// - 구르기를 하면 짧은 시간 동안 아주 빠르게 한 방향으로 나가요.
// - 벽에 부딪히면 벽을 따라 미끄러져요 (tile_collision이 도와줘요).
//
// 게임 규칙 쪽 코드라서 SDL(그리기 도구)을 전혀 몰라요. 그래서 창 없이 테스트할 수 있어요.
// 정해진 박자(1/60초)마다 step()을 한 번씩 불러요.
// 화면은 바로 전 박자 위치와 지금 위치 사이를 이어서(보간) 그려서 부드럽게 보여요.

#include "engine/core/math.hpp"
#include "game/entities/movement_tuning.hpp"

#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

#include <cstdint>

namespace wind::game {

class Mover {
public:
    // 지금 무엇을 하고 있나
    enum class State : std::uint8_t { Idle, Walking, Running, Dodging };

    Mover(engine::Vec2 feet, world::Direction facing, MovementTuning tuning);

    // 한 박자 움직여요.
    // move_input: 가고 싶은 방향 (길이 0~1), dodge: 이번 박자에 구르기를 시작할지
    // run: 달리기(더 빠른 최고 속도). 스태미나가 있는지는 부르는 쪽(PlayerMotion)이 확인해요.
    // 구르기를 실제로 시작했으면 true를 돌려줘요.
    bool step(double dt_seconds, engine::Vec2 move_input, bool dodge, const world::MapData& map,
              bool run = false);

    // 지금 구르기를 시작할 수 있나요? (구르는 중이 아니고, 쉬는 시간도 끝났으면)
    [[nodiscard]] bool can_dodge() const;
    // 순간이동: 미끄러지는 중간 모습 없이 바로 그 자리에 서요.
    void teleport(engine::Vec2 feet);
    // 바라보는 방향을 바로 바꿔요 (공격을 시작할 때 누른 방향으로 돌아서요).
    void face(world::Direction facing) { facing_ = facing; }

    [[nodiscard]] engine::Vec2 feet() const { return feet_; }
    // 화면에 그릴 위치: 전 박자 위치와 지금 위치 사이 (alpha = 0이면 전, 1이면 지금)
    [[nodiscard]] engine::Vec2 interpolated_feet(float alpha) const;
    [[nodiscard]] engine::Vec2 velocity() const { return velocity_; }
    [[nodiscard]] float speed() const { return engine::length(velocity_); }
    [[nodiscard]] world::Direction facing() const { return facing_; }
    [[nodiscard]] State state() const { return state_; }
    // 구르기 처음 몇 초 동안은 공격에 맞지 않아요 (단계 4에서 써요).
    [[nodiscard]] bool invulnerable() const;
    [[nodiscard]] const MovementTuning& tuning() const { return tuning_; }

private:
    void start_dodge(engine::Vec2 move_input);
    void update_velocity(float dt, engine::Vec2 move_input, bool run);
    void update_facing(engine::Vec2 move_input);

    MovementTuning tuning_;
    engine::Vec2 feet_;
    engine::Vec2 previous_feet_;
    engine::Vec2 velocity_;
    world::Direction facing_;
    State state_ = State::Idle;
    engine::Vec2 dodge_direction_;
    float dodge_elapsed_ = 0.0f;  // 구르기 시작 뒤 흐른 시간
    float dodge_cooldown_ = 0.0f;  // 다시 구를 수 있을 때까지 남은 시간
};

}  // namespace wind::game
