// 이 파일은 무엇을 하나요?
// 지연 바의 꼬리 따라가기, 차오르기, 자동 숨김 규칙이에요.

#include "game/ui/delayed_bar.hpp"

#include <algorithm>

namespace wind::game {

DelayedBar::DelayedBar(DelayedBarTuning tuning, float start)
    : tuning_(tuning),
      target_(std::clamp(start, 0.0f, 1.0f)),
      value_(target_),
      trail_(target_) {}

void DelayedBar::set(float fraction) {
    const float next = std::clamp(fraction, 0.0f, 1.0f);
    if (next < target_) {
        // 줄었어요: 진짜 바는 바로 줄고, 꼬리는 지금 자리에서 잠깐 기다려요.
        // (계속 조금씩 줄 때는 기다리는 시간을 처음부터 다시 세서, 꼬리가 한 덩어리로 남아요)
        trail_ = std::max(trail_, value_);
        value_ = next;
        trail_wait_ = tuning_.trail_delay_seconds;
    }
    target_ = next;
}

void DelayedBar::update(float dt_seconds) {
    if (dt_seconds <= 0.0f) {
        return;
    }
    // 늘 때는 부드럽게 차올라요.
    if (value_ < target_) {
        value_ = std::min(target_, value_ + tuning_.fill_speed * dt_seconds);
    }
    // 꼬리: 기다렸다가 진짜 바를 따라 줄어요. 진짜 바보다 짧아지지는 않아요.
    if (trail_wait_ > 0.0f) {
        trail_wait_ = std::max(0.0f, trail_wait_ - dt_seconds);
    } else {
        trail_ = std::max(value_, trail_ - tuning_.trail_speed * dt_seconds);
    }
    trail_ = std::max(trail_, value_);

    // 자동 숨김: 가득 차 있으면 시간을 세서 흐려지고, 아니면 바로 진해져요.
    if (!tuning_.auto_hide) {
        alpha_ = 1.0f;
        return;
    }
    if (value_ >= 1.0f && trail_ >= 1.0f) {
        full_for_ += dt_seconds;
    } else {
        full_for_ = 0.0f;
        alpha_ = 1.0f;
    }
    if (full_for_ >= tuning_.hide_after_seconds) {
        alpha_ = std::max(tuning_.hidden_alpha, alpha_ - tuning_.fade_speed * dt_seconds);
    }
}

}  // namespace wind::game
