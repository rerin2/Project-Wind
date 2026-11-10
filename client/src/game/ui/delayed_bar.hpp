#pragma once

// 이 파일은 무엇을 하나요?
// HUD 지연 바예요. (명세 6.9 "지연 바", 13.2) HP·MP·스태미나 바가 모두 이걸 써요.
// - 값이 줄면: 진짜 바는 바로 줄고, 줄어든 만큼은 밝은 색 "꼬리"로 잠깐(0.35초) 남았다가 따라 줄어요.
//   그래서 "방금 얼마나 줄었는지"가 눈에 보여요.
// - 값이 늘면: 바가 부드럽게 차올라요.
// - 자동 숨김(선택): 가득 찬 채로 1.2초가 지나면 흐려지고, 다시 쓰기 시작하면 바로 진해져요.
// 그리지 않고 길이(0~1)와 투명도만 계산해서 창 없이 테스트할 수 있어요. 보여 주기 전용이에요.

namespace wind::game {

struct DelayedBarTuning {
    float trail_delay_seconds = 0.35f;  // 꼬리가 따라 줄기 시작할 때까지
    float trail_speed = 1.2f;           // 꼬리가 줄어드는 빠르기 (바 전체 = 1, 1초에)
    float fill_speed = 2.5f;            // 늘 때 차오르는 빠르기 (1초에)
    bool auto_hide = false;             // 가득 차면 흐려지기
    float hide_after_seconds = 1.2f;    // 가득 찬 뒤 흐려지기 시작할 때까지
    float hidden_alpha = 0.3f;          // 흐려졌을 때 투명도
    float fade_speed = 2.0f;            // 흐려지는 빠르기 (1초에)
};

class DelayedBar {
public:
    explicit DelayedBar(DelayedBarTuning tuning = {}, float start = 1.0f);

    // 새 값(0~1). 범위 밖이면 잘라요.
    void set(float fraction);
    void update(float dt_seconds);

    [[nodiscard]] float value() const { return value_; }  // 진짜 바 길이
    [[nodiscard]] float trail() const { return trail_; }  // 꼬리 끝 (value 이상)
    [[nodiscard]] float alpha() const { return alpha_; }  // 투명도

private:
    DelayedBarTuning tuning_;
    float target_;
    float value_;
    float trail_;
    float trail_wait_ = 0.0f;
    float full_for_ = 0.0f;  // 가득 찬 채로 지난 시간
    float alpha_ = 1.0f;
};

}  // namespace wind::game
