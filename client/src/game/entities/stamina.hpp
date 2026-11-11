#pragma once

// 이 파일은 무엇을 하나요?
// 스태미나(기력) 담당이에요. (명세 7.2.1)
// 달리기와 구르기를 하면 줄고, 안 쓰고 잠깐 쉬면 다시 차요.
// 0까지 다 쓰면 "지침" 상태가 돼서, 어느 정도(기본 25) 다시 찰 때까지 달리기·구르기를 못 해요.
// 지침이 없으면 0 근처에서 달리기가 켜졌다 꺼졌다 덜덜거려요. 그걸 막는 장치예요.
// 게임 규칙이라 정해진 박자(1/60초)마다 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.

namespace wind::game {

// 스태미나 숫자 모음 (명세 7.2.1 기본값). 값의 이유는 docs/GAMEPLAY.md "스태미나" 절에 있어요.
struct StaminaTuning {
    float max = 100.0f;                 // 최대 스태미나 (나중에 지구력 능력치로 바뀌어요)
    float run_cost_per_second = 25.0f;  // 달리는 동안 1초에 쓰는 양
    float dodge_cost = 25.0f;           // 구르기 한 번에 쓰는 양
    float regen_delay_seconds = 0.6f;   // 마지막으로 쓴 뒤 이만큼 지나야 다시 차기 시작
    float regen_per_second = 30.0f;     // 다시 찰 때 1초에 차는 양
    float exhausted_until = 25.0f;      // 지침 상태는 이만큼 찰 때까지 이어져요
};

class Stamina {
public:
    explicit Stamina(StaminaTuning tuning);

    // 달릴 수 있나요? (지침이 아니고 조금이라도 남아 있으면)
    [[nodiscard]] bool can_run() const;
    // cost만큼 한 번에 쓰는 동작(구르기)을 할 수 있나요? (지침이 아니고 cost 이상 남아 있으면)
    [[nodiscard]] bool can_afford(float cost) const;
    // 무기 공격을 할 수 있나요? (명세 7.2.1, 소울라이크식)
    // 지침이 아니고 조금이라도 남아 있으면 돼요. 모자라면 공격은 나가고 0이 돼요.
    [[nodiscard]] bool can_attack() const;
    // amount만큼 써요. 0 아래로는 안 내려가고, 0이 되면 지침 상태가 돼요.
    // 쓰면 "다시 차기 전 기다리는 시간"이 처음부터 다시 시작돼요.
    void spend(float amount);
    // 한 박자: 기다리는 시간이 지났으면 조금씩 다시 채워요. 최대를 넘지 않아요.
    void update(float dt_seconds);
    // 가득 채워요 (쉬기, 부활).
    void refill();

    [[nodiscard]] float current() const { return current_; }
    [[nodiscard]] float max() const { return tuning_.max; }
    // 0~1 (바 길이용)
    [[nodiscard]] float fraction() const;
    [[nodiscard]] bool exhausted() const { return exhausted_; }
    [[nodiscard]] const StaminaTuning& tuning() const { return tuning_; }

private:
    StaminaTuning tuning_;
    float current_;
    float regen_wait_ = 0.0f;  // 다시 차기 시작할 때까지 남은 시간
    bool exhausted_ = false;
};

}  // namespace wind::game
