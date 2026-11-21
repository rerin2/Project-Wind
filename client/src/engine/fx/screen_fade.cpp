// 이 파일은 무엇을 하나요?
// 페이드 단계(밝음 -> 어두워지기 -> 까맘 -> 밝아지기 -> 밝음)를 넘기는 곳이에요.

#include "engine/fx/screen_fade.hpp"

#include <cmath>

namespace wind::engine {

ScreenFade::ScreenFade(float seconds) : seconds_(seconds), trip_seconds_(seconds) {}

void ScreenFade::fade_out(world::ScreenTransition style, float seconds) {
    style_ = style;
    // 이번 전환 길이: 따로 정했으면 그 값, 아니면 기본 길이 (0·음수·NaN은 기본)
    trip_seconds_ = (std::isfinite(seconds) && seconds > 0.0f) ? seconds : seconds_;
    // 어두워질 때는 처음엔 천천히, 끝에 빠르게 (InQuad)
    tween_ = Tween(darkness(), 1.0f, trip_seconds_, Ease::InQuad);
    phase_ = Phase::FadingOut;
}

void ScreenFade::fade_in() {
    // 밝아질 때는 처음엔 빠르게, 끝에 사뿐히 (OutQuad). 어두워질 때와 같은 길이예요.
    tween_ = Tween(darkness(), 0.0f, trip_seconds_, Ease::OutQuad);
    phase_ = Phase::FadingIn;
}

void ScreenFade::update(float dt_seconds) {
    if (phase_ == Phase::FadingOut || phase_ == Phase::FadingIn) {
        tween_.update(dt_seconds);
        if (tween_.finished()) {
            phase_ = phase_ == Phase::FadingOut ? Phase::Covered : Phase::Clear;
        }
    }
}

float ScreenFade::darkness() const {
    switch (phase_) {
        case Phase::Clear: return 0.0f;
        case Phase::Covered: return 1.0f;
        case Phase::FadingOut:
        case Phase::FadingIn: return tween_.value();
    }
    return 0.0f;
}

}  // namespace wind::engine
