// 이 파일은 무엇을 하나요?
// 지역 이름 배너의 나타나기 -> 머물기 -> 사라지기 순서를 넘기는 곳이에요.

#include "game/ui/area_banner.hpp"

namespace wind::game {

void AreaBanner::show(std::string_view name) {
    name_ = std::string(name);
    if (name_.empty()) {
        phase_ = Phase::Hidden;
        return;
    }
    enter(Phase::In);
}

void AreaBanner::enter(Phase phase) {
    phase_ = phase;
    switch (phase) {
        // 나타날 때는 끝에서 사뿐히(OutCubic), 사라질 때는 처음엔 천천히(InQuad)
        case Phase::In: tween_ = engine::Tween(0.0f, 1.0f, kInSeconds, engine::Ease::OutCubic); break;
        case Phase::Hold: hold_left_ = kHoldSeconds; break;
        case Phase::Out: tween_ = engine::Tween(1.0f, 0.0f, kOutSeconds, engine::Ease::InQuad); break;
        case Phase::Hidden: break;
    }
}

void AreaBanner::update(float dt_seconds) {
    switch (phase_) {
        case Phase::In:
            tween_.update(dt_seconds);
            if (tween_.finished()) enter(Phase::Hold);
            break;
        case Phase::Hold:
            hold_left_ -= dt_seconds;
            if (hold_left_ <= 0.0f) enter(Phase::Out);
            break;
        case Phase::Out:
            tween_.update(dt_seconds);
            if (tween_.finished()) phase_ = Phase::Hidden;
            break;
        case Phase::Hidden: break;
    }
}

float AreaBanner::alpha() const {
    switch (phase_) {
        case Phase::In:
        case Phase::Out: return tween_.value();
        case Phase::Hold: return 1.0f;
        case Phase::Hidden: return 0.0f;
    }
    return 0.0f;
}

float AreaBanner::rise() const {
    // 나타날 때만 위에서 내려와요. 사라질 때는 제자리에서 흐려져요.
    return phase_ == Phase::In ? (1.0f - tween_.value()) * kRisePixels : 0.0f;
}

}  // namespace wind::game
