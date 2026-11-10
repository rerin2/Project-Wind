#pragma once

// 이 파일은 무엇을 하나요?
// 지역 이름 배너예요. (명세 6.9, 단계 8)
// 새 지역에 들어가면 화면 위쪽에 "시험 초원" 같은 이름이
// 1) 위에서 살짝 내려오며 나타나고(0.5초)  2) 잠깐 머물고(2.2초)  3) 천천히 사라져요(0.7초).
// 그리지는 않고 "지금 얼마나 보이는지(투명도)"와 "얼마나 내려왔는지"만 계산해요.
// 보여 주기 전용이라 게임 규칙에는 영향이 없어요.

#include "engine/tween/tween.hpp"

#include <string>
#include <string_view>

namespace wind::game {

class AreaBanner {
public:
    // 새 이름으로 처음부터 다시 보여 줘요. 빈 이름이면 아무것도 안 보여요.
    void show(std::string_view name);
    void update(float dt_seconds);

    [[nodiscard]] bool visible() const { return phase_ != Phase::Hidden; }
    [[nodiscard]] const std::string& name() const { return name_; }
    // 투명도 0(안 보임) ~ 1(다 보임)
    [[nodiscard]] float alpha() const;
    // 위로 몇 픽셀 올라가 있는지 (나타날 때 8 -> 0)
    [[nodiscard]] float rise() const;

    static constexpr float kInSeconds = 0.5f;
    static constexpr float kHoldSeconds = 2.2f;
    static constexpr float kOutSeconds = 0.7f;
    static constexpr float kRisePixels = 8.0f;

private:
    enum class Phase { Hidden, In, Hold, Out };

    void enter(Phase phase);

    std::string name_;
    Phase phase_ = Phase::Hidden;
    engine::Tween tween_;
    float hold_left_ = 0.0f;
};

}  // namespace wind::game
