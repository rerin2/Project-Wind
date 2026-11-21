// 이 파일은 무엇을 하나요?
// 쓰러짐 연출 시계를 흘리고, 흑백 정도와 눕는 정도를 계산하는 곳이에요.

#include "game/presentation/death_fx.hpp"

#include "engine/tween/easing.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {

void DeathFx::update(float dt_seconds, bool dead) {
    if (!dead) {  // 서 있으면 처음 상태로
        dead_ = false;
        seconds_ = 0.0f;
        return;
    }
    dead_ = true;
    if (std::isfinite(dt_seconds) && dt_seconds > 0.0f) {
        seconds_ += dt_seconds;  // 쓰러진 뒤 흐른 시간
    }
}

float DeathFx::grayscale() const {
    if (!dead_) return 0.0f;
    // 빨리 회색이 되기 시작해서 끝은 천천히 (OutQuad): 쓰러진 순간 바로 "아차" 하는 느낌
    return engine::ease(engine::Ease::OutQuad, seconds_ / kGrayscaleSeconds);
}

float DeathFx::lying() const {
    if (!dead_) return 0.0f;
    // 처음엔 천천히 기울다가 바닥에 털썩 (InQuad)
    return engine::ease(engine::Ease::InQuad, seconds_ / kLyingSeconds);
}

}  // namespace wind::game
