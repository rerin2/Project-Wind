#pragma once

// 이 파일은 무엇을 하나요?
// 주인공이 쓰러졌을 때의 연출 시계예요. (명세 9.5, 12b)
// - 쓰러져 있는 동안 시계가 흘러요 (화면 시간). 다시 서면(새 맵) 0으로 돌아가요.
// - grayscale(): 화면이 흑백이 되는 정도 0~1. 1.5초 동안 서서히 (슬로모션과 같은 길이)
// - lying(): 주인공이 옆으로 눕는 정도 0~1. 0.35초 동안, 처음엔 천천히 기울다가 털썩(InQuad)
// 보여 주기만 하고 게임 결과는 바꾸지 않아요. 값의 이유는 docs/VISUALS.md "Hero falling" 절에 있어요.

namespace wind::game {

class DeathFx {
public:
    // 화면 한 장마다. dead: 주인공이 지금 쓰러져 있나
    void update(float dt_seconds, bool dead);

    [[nodiscard]] bool active() const { return dead_; }
    // 쓰러진 뒤 흐른 화면 시간 (초)
    [[nodiscard]] float seconds() const { return seconds_; }
    // 흑백 정도 0(색 그대로) ~ 1(완전 흑백)
    [[nodiscard]] float grayscale() const;
    // 눕는 정도 0(서 있음) ~ 1(다 누움)
    [[nodiscard]] float lying() const;

    static constexpr float kGrayscaleSeconds = 1.5f;  // 흑백이 다 되기까지
    static constexpr float kLyingSeconds = 0.35f;     // 다 눕기까지

private:
    bool dead_ = false;
    float seconds_ = 0.0f;
};

}  // namespace wind::game
