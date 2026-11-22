// 이 파일은 무엇을 하나요?
// 룬 숫자 굴리기와 "+N" 떠오르기.

#include "game/ui/rune_counter.hpp"

#include "engine/tween/easing.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kGainRollSeconds = 0.6f;   // 늘 때 굴리는 시간
constexpr float kLossRollSeconds = 0.8f;   // 줄 때 (조금 더 천천히, 아프게)
constexpr float kPopupSeconds = 1.2f;      // "+N"이 떠 있는 시간
constexpr float kPopupFade = 0.4f;         // 끝에서 흐려지는 시간
constexpr float kPopupRise = 10.0f;        // 떠오르는 높이 (픽셀)

}  // namespace

void RuneCounter::update(float dt_seconds, std::int64_t actual) {
    if (!started_) {  // 처음 본 값은 그냥 보여 줘요
        started_ = true;
        from_ = to_ = actual;
        roll_seconds_ = roll_length_ = 0.0f;
        return;
    }
    roll_seconds_ = std::min(roll_length_, roll_seconds_ + dt_seconds);
    popup_seconds_ = std::max(0.0f, popup_seconds_ - dt_seconds);
    if (actual == to_) return;  // 바뀐 게 없어요
    const std::int64_t change = actual - to_;
    from_ = shown();            // 지금 보이는 곳에서 새로 굴리기 시작
    to_ = actual;
    roll_seconds_ = 0.0f;
    roll_length_ = change > 0 ? kGainRollSeconds : kLossRollSeconds;
    // 같은 쪽으로 또 바뀌면 떠 있는 숫자에 더해요 (슬라임 둘을 연달아 잡으면 +24)
    const bool same_side = popup_seconds_ > 0.0f && (popup_amount_ > 0) == (change > 0);
    popup_amount_ = same_side ? popup_amount_ + change : change;
    popup_seconds_ = kPopupSeconds;
}

std::int64_t RuneCounter::shown() const {
    if (!rolling() || roll_length_ <= 0.0f) return to_;
    const float t = engine::ease(engine::Ease::OutCubic, roll_seconds_ / roll_length_);  // 빠르게 시작, 천천히 멈춤
    const double value = static_cast<double>(from_) + static_cast<double>(to_ - from_) * t;
    return static_cast<std::int64_t>(std::llround(value));
}

float RuneCounter::popup_alpha() const {
    return std::clamp(popup_seconds_ / kPopupFade, 0.0f, 1.0f);
}

float RuneCounter::popup_rise() const {
    return kPopupRise * (1.0f - popup_seconds_ / kPopupSeconds);  // 0에서 위로
}

}  // namespace wind::game
