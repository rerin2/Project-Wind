// 이 파일은 무엇을 하나요?
// "쓰러졌다" 화면의 기다리기 -> 나타나기 -> 안내 순서를 시간으로 계산하는 곳이에요.

#include "game/ui/fallen_screen.hpp"

#include "engine/tween/easing.hpp"

#include <cmath>

namespace wind::game {

void FallenScreen::update(float dt_seconds, bool down) {
    if (!down) {  // 서 있으면 처음으로
        down_ = false;
        seconds_ = 0.0f;
        return;
    }
    down_ = true;
    if (std::isfinite(dt_seconds) && dt_seconds > 0.0f) {
        seconds_ += dt_seconds;
    }
}

float FallenScreen::alpha() const {
    if (!down_) return 0.0f;
    // 기다린 뒤 0.8초 동안 사뿐히 나타나요 (OutCubic). ease가 0~1로 잘라 줘요.
    return engine::ease(engine::Ease::OutCubic, (seconds_ - kDelaySeconds) / kInSeconds);
}

float FallenScreen::drop() const {
    return (1.0f - alpha()) * kDropPixels;  // 덜 나타났을수록 아래에 있어요
}

float FallenScreen::hint_alpha() const {
    if (!down_) return 0.0f;
    return engine::ease(engine::Ease::OutQuad, (seconds_ - kDelaySeconds - kInSeconds) / kHintSeconds);
}

bool FallenScreen::ready() const {
    return down_ && seconds_ >= kDelaySeconds + kInSeconds;
}

}  // namespace wind::game
