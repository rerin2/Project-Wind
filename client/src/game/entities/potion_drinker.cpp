// 이 파일은 무엇을 하나요?
// 물약 마시기 시간표: 시작 -> 꿀꺽(효과 한 번) -> 병 내리기 -> 끝. 맞으면 끊겨요.

#include "game/entities/potion_drinker.hpp"

namespace wind::game {

void PotionDrinker::start(const items::UseEffect& effect) {
    effect_ = effect;
    elapsed_ = 0.0f;
    gulped_ = false;
}

std::optional<items::UseEffect> PotionDrinker::step(float dt_seconds) {
    if (!effect_) return std::nullopt;
    elapsed_ += dt_seconds;
    std::optional<items::UseEffect> out;
    if (!gulped_ && elapsed_ >= kGulpSeconds) {  // 꿀꺽: 효과는 마시기마다 한 번만
        gulped_ = true;
        out = effect_;
    }
    if (elapsed_ >= kDrinkSeconds) effect_.reset();  // 다 마셨어요
    return out;
}

void PotionDrinker::cancel() {
    effect_.reset();  // 꿀꺽 전이면 효과도 같이 사라져요 (물약은 이미 썼어요)
    elapsed_ = 0.0f;
    gulped_ = false;
}

}  // namespace wind::game
