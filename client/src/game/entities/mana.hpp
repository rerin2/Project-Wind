#pragma once

// 이 파일은 무엇을 하나요?
// MP(정신력) 통이에요. (명세 8.2, 8.6)
// - 마술을 쓰면 줄고, 1초에 2씩 천천히 다시 차요.
// - 스태미나와 달리 "기다렸다 차기"와 "지침"이 없어요. 쓰는 동안에도 계속 차요.
// - 쓰기는 "전부 아니면 전부 안 함": 모자라면 하나도 안 줄고 실패해요.
//   그래서 MP가 모자란 마술은 나가지 않아요(9e에서 시전이 이 규칙을 써요).
// 게임 규칙이라 정해진 박자마다 계산하고, SDL을 몰라서 창 없이 테스트할 수 있어요.

namespace wind::game {

// MP 숫자 모음. 값의 이유는 docs/GAMEPLAY.md "MP" 절에 있어요.
struct ManaTuning {
    float max = 50.0f;              // 최대 MP (9단계는 고정, 13단계부터 정신력 곡선으로 바뀌어요)
    float regen_per_second = 2.0f;  // 1초에 다시 차는 양
};

class Mana {
public:
    explicit Mana(ManaTuning tuning);

    // cost만큼 쓸 수 있나요? (지금 MP가 cost 이상이면 돼요)
    [[nodiscard]] bool can_afford(float cost) const;
    // cost만큼 써요. 모자라면 아무것도 바꾸지 않고 false예요. 음수·0은 쓴 셈 치고 true예요.
    bool try_spend(float cost);
    // 한 박자: 최대를 넘지 않게 조금씩 채워요.
    void update(float dt_seconds);
    // 가득 채워요 (쉬기, 부활).
    void refill();

    [[nodiscard]] float current() const { return current_; }
    [[nodiscard]] float max() const { return tuning_.max; }
    // 0~1 (바 길이용)
    [[nodiscard]] float fraction() const;
    [[nodiscard]] const ManaTuning& tuning() const { return tuning_; }

private:
    ManaTuning tuning_;
    float current_;
};

}  // namespace wind::game
