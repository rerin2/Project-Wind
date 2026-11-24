// 이 파일은 무엇을 하나요?
// 저장 아이콘의 시간 흐름: 저장 횟수가 늘면 나타나고, 잠깐 보였다가 사라져요.

#include "game/ui/save_icon.hpp"

#include "engine/tween/easing.hpp"

#include <algorithm>

namespace wind::game {

void SaveIcon::update(float dt_seconds, unsigned saves_done) {
    const float dt = std::max(dt_seconds, 0.0f);
    if (!started_) {  // 처음 본 횟수는 기준으로만 (켜자마자 뜨지 않게)
        started_ = true;
        last_count_ = saves_done;
        return;
    }
    if (saves_done != last_count_) {  // 새 저장이 있었어요
        last_count_ = saves_done;
        if (!active_) {               // 숨어 있었으면 처음부터 나타나요
            active_ = true;
            fade_in_ = 0.0f;
            shown_ = 0.0f;
        }
        since_save_ = 0.0f;           // 보이는 시간은 마지막 저장부터 다시 세요
        return;                       // 이번 장은 0초째 (dt는 다음 장부터)
    }
    if (!active_) return;
    since_save_ += dt;
    fade_in_ += dt;
    shown_ += dt;
    if (since_save_ >= kHoldSeconds + kFadeOutSeconds) active_ = false;  // 다 사라졌어요
}

float SaveIcon::alpha() const {
    if (!active_) return 0.0f;
    // 나타나기: 사뿐히 멈추며 0 -> 1
    const float in = engine::ease(engine::Ease::OutCubic, fade_in_ / kFadeInSeconds);
    // 사라지기: 보이는 시간이 지나면 천천히 출발해 1 -> 0
    const float out_t = (since_save_ - kHoldSeconds) / kFadeOutSeconds;
    const float out = 1.0f - engine::ease(engine::Ease::InCubic, out_t);
    return std::clamp(std::min(in, out), 0.0f, 1.0f);
}

}  // namespace wind::game
