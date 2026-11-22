// 이 파일은 무엇을 하나요?
// 쉬기 시간표: 옮겨 가기 -> 앉기 -> (요청하면) 기지개 -> 끝.

#include "game/entities/hero_rest.hpp"

#include <algorithm>

namespace wind::game {

HeroRest::HeroRest(RestTuning tuning) : tuning_(tuning) {}

void HeroRest::enter(Phase phase) {
    phase_ = phase;
    phase_seconds_ = 0.0f;  // 새 단계는 0초부터
}

void HeroRest::begin(engine::Vec2 seat) {
    seat_ = seat;
    enter(Phase::Settling);
}

void HeroRest::sit_now(engine::Vec2 seat) {
    seat_ = seat;
    enter(Phase::Sitting);
}

bool HeroRest::stand_up() {
    if (phase_ != Phase::Sitting) return false;  // 앉아 있을 때만 일어나요
    enter(Phase::Standing);
    return true;
}

engine::Vec2 HeroRest::step(float dt_seconds, engine::Vec2 feet) {
    if (phase_ == Phase::None || !(dt_seconds > 0.0f)) return {};  // 히트스톱(0)이면 멈춰 있어요
    phase_seconds_ += dt_seconds;
    switch (phase_) {
        case Phase::Settling: {
            // 남은 시간 동안 남은 거리를 똑같이 나눠 가요. 마지막 박자에 딱 자리에 닿아요.
            const float left = tuning_.settle_seconds - (phase_seconds_ - dt_seconds);  // 이 박자 전 남은 시간
            const engine::Vec2 to_seat = seat_ - feet;
            if (phase_seconds_ >= tuning_.settle_seconds) {
                enter(Phase::Sitting);
                return to_seat;  // 남은 거리를 다 가요
            }
            return to_seat * std::clamp(dt_seconds / std::max(left, dt_seconds), 0.0f, 1.0f);
        }
        case Phase::Standing:
            if (phase_seconds_ >= tuning_.stand_seconds) enter(Phase::None);  // 다 일어났어요
            return {};
        case Phase::Sitting:
        case Phase::None:
            return {};
    }
    return {};
}

}  // namespace wind::game
