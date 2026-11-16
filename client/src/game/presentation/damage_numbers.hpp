#pragma once

// 이 파일은 무엇을 하나요?
// 피해 숫자예요. (명세 6.7) 맞은 곳에서 숫자가 톡 튀어 오르고, 잠깐 머물다 사라져요.
// - 치명타는 더 높이 튀고, 더 오래 보이고, 그리는 쪽이 금색 굵은 글씨에 "!"를 붙여요.
// - 연달아 맞으면 숫자가 겹치지 않게 좌우로 조금씩 번갈아 놓아요.
// 여기서는 "어디에, 얼마나 진하게"만 계산해요. 글자를 그리는 건 app 층(damage_number_drawing)이 해요.
// 보여 주기 전용이라 게임 결과를 바꾸지 않아요.

#include "engine/core/math.hpp"

#include <cstddef>
#include <vector>

namespace wind::game {

// 떠 있는 숫자 하나
struct DamageNumber {
    engine::Vec2 at;        // 처음 생긴 곳 (맵 위, 맞은 상자 가운데)
    int amount = 0;         // 피해량
    bool critical = false;  // 치명타
    float side = 0.0f;      // 좌우로 비킨 거리 (픽셀)
    float age = 0.0f;       // 생긴 뒤 지난 시간 (초)
};

// 숫자 움직임 값. 이유는 docs/VISUALS.md "Hit feel" 절에 있어요.
struct DamageNumberTuning {
    float life = 0.7f;            // 보이는 시간 (초)
    float crit_life = 0.9f;
    float rise = 14.0f;           // 튀어 오르는 높이 (픽셀)
    float crit_rise = 20.0f;
    float fade_from = 0.6f;       // 수명의 60%부터 흐려져요
    float side_step = 5.0f;       // 연달아 나오면 좌우로 비키는 거리
    std::size_t max_numbers = 32; // 한꺼번에 떠 있는 숫자 최대 (넘으면 가장 오래된 것부터 지워요)
};

class DamageNumbers {
public:
    explicit DamageNumbers(DamageNumberTuning tuning = {});

    // 맞은 곳 at에 숫자를 하나 띄워요
    void spawn(engine::Vec2 at, int amount, bool critical);
    // 화면 한 장마다: 나이를 먹이고, 다 산 숫자는 지워요
    void update(float dt_seconds);

    [[nodiscard]] const std::vector<DamageNumber>& numbers() const { return numbers_; }
    // 지금 그릴 위치 (맵 위). 처음엔 빠르게 오르고 꼭대기에서 멈춰요
    [[nodiscard]] engine::Vec2 position(const DamageNumber& number) const;
    // 지금 진하기 0~1
    [[nodiscard]] float alpha(const DamageNumber& number) const;

private:
    [[nodiscard]] float life_of(const DamageNumber& number) const;

    DamageNumberTuning tuning_;
    std::vector<DamageNumber> numbers_;
    unsigned spawned_ = 0;  // 지금까지 띄운 수 (좌우 번갈아 놓기용)
};

}  // namespace wind::game
