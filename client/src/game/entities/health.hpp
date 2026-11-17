#pragma once

// 이 파일은 무엇을 하나요?
// 주인공 HP(체력) 통이에요. (명세 8.2, 9.4, 11c)
// - 맞으면 줄어요. 줄어든 만큼(실제로 깎인 양)을 돌려줘요.
// - 임시 규칙(11c, 개발자와 정함): 사망·부활은 12단계라서, 그 전까지 HP는 floor(1) 아래로 내려가지 않아요.
//   12단계에서 floor를 0으로 바꾸고 0이 되면 쓰러지게 해요.
// 게임 규칙이라 정해진 박자마다 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.

namespace wind::game {

// HP 숫자 모음. 값의 이유는 docs/GAMEPLAY.md "Hero getting hit" 절에 있어요.
struct HealthTuning {
    int max = 100;  // 최대 HP (명세 8.2: 생명력 10이면 100. 13단계부터 곡선에서 구해요)
    int floor = 1;  // 이보다 낮아지지 않아요 (11c 임시: 사망은 12단계)
};

class Health {
public:
    explicit Health(HealthTuning tuning);

    // amount만큼 깎아요 (floor 아래로는 안 내려가요). 실제로 깎인 양을 돌려줘요. 0 이하는 무시해요.
    int take(int amount);
    // 가득 채워요 (쉬기, 부활).
    void refill() { current_ = tuning_.max; }

    [[nodiscard]] int current() const { return current_; }
    [[nodiscard]] int max() const { return tuning_.max; }
    // 0~1 (바 길이용, 12단계 HUD)
    [[nodiscard]] float fraction() const;

private:
    HealthTuning tuning_;
    int current_;
};

}  // namespace wind::game
