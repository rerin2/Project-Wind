#pragma once

// 이 파일은 무엇을 하나요?
// 훈련용 허수아비 하나예요. (9단계 9c)
// - HP가 있고, 맞으면 줄어요.
// - HP가 0이 되면 쓰러지고(이때는 안 맞아요), 정해진 시간(기본 1.5초) 뒤 HP가 가득 차서 다시 서요.
// - 맞는 상자(허트박스)는 발 위치를 기준으로 데이터에 적힌 대로 놓여요.
// - 맞으면 몸통이 맞은 쪽으로 살짝 밀렸다가 용수철처럼 돌아와요 (KnockSway, 10a).
//   발 위치(칸)는 그대로이고, 밀린 거리만큼 맞는 상자와 그림이 같이 움직여요.
// 게임 규칙이라 고정 박자로 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/attack_runner.hpp"
#include "game/combat/knock_sway.hpp"

#include "wind/combat/training_dummy_data.hpp"

#include <string>

namespace wind::game {

class TrainingDummy {
public:
    // id: 맞는 대상 번호 (같은 휘두르기에 두 번 맞지 않게 구별해요), name: 맵 안 이름(로그용)
    TrainingDummy(TargetId id, std::string name, combat::TrainingDummyData data, engine::Vec2 feet);

    // 지금 맞을 수 있나요? (쓰러져 있으면 안 맞아요)
    [[nodiscard]] bool can_be_hit() const { return !down(); }
    // 피해를 받아요. 실제로 깎인 양을 돌려줘요 (쓰러져 있으면 0). HP가 0이 되면 쓰러져요.
    int take_damage(int amount);
    // 맞은 방향으로 knockback(픽셀/초)만큼 몸통을 밀어요 (용수철 흔들림)
    void push(engine::Vec2 direction, float knockback) { sway_.push(direction, knockback); }
    // 한 박자: 흔들림을 움직이고, 쓰러져 있으면 시간을 세고, 다 되면 HP를 채워 다시 세워요.
    void step(float dt_seconds);

    // 맵 위의 맞는 상자 (픽셀, 밀린 만큼 같이 움직여요)
    [[nodiscard]] engine::RectF hurtbox() const;
    // 몸통이 제자리에서 밀린 거리 (그림도 이만큼 옮겨 그려요)
    [[nodiscard]] engine::Vec2 body_offset() const { return sway_.offset(); }
    [[nodiscard]] TargetId id() const { return id_; }
    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] int hp() const { return hp_; }
    [[nodiscard]] int max_hp() const { return data_.max_hp; }
    [[nodiscard]] float defense() const { return data_.defense; }
    [[nodiscard]] bool down() const { return down_left_ > 0.0f; }
    // 쓰러졌나요? (판정 규칙이 몬스터와 같은 이름으로 물어봐요, 11d)
    [[nodiscard]] bool defeated() const { return down(); }
    [[nodiscard]] engine::Vec2 feet() const { return feet_; }
    [[nodiscard]] const combat::TrainingDummyData& data() const { return data_; }

private:
    TargetId id_;                     // 맞는 대상 번호
    std::string name_;                // 맵 안 이름 ("meadow_dummy_1")
    combat::TrainingDummyData data_;  // HP·방어·상자·그림 설명
    engine::Vec2 feet_;               // 서 있는 발 위치 (움직이지 않아요)
    int hp_;                          // 지금 HP
    float down_left_ = 0.0f;          // 쓰러져 있을 남은 시간 (0이면 서 있음)
    KnockSway sway_;                  // 맞으면 몸통이 밀렸다 돌아오는 용수철
};

}  // namespace wind::game
