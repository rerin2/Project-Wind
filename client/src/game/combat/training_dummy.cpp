// 이 파일은 무엇을 하나요?
// 허수아비가 피해를 받고, 쓰러지고, 다시 서는 규칙이에요.

#include "game/combat/training_dummy.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {

TrainingDummy::TrainingDummy(TargetId id, std::string name, combat::TrainingDummyData data,
                             engine::Vec2 feet)
    : id_(id), name_(std::move(name)), data_(std::move(data)), feet_(feet), hp_(data_.max_hp) {}

int TrainingDummy::take_damage(int amount) {
    if (down() || amount <= 0) return 0;          // 쓰러져 있거나 0 이하 피해는 무시
    const int applied = std::min(amount, hp_);    // HP보다 많이는 못 깎아요
    hp_ -= applied;                               // HP 깎기
    if (hp_ <= 0) {                               // 0이 되면
        down_left_ = data_.recover_seconds;       // 쓰러져서 다시 서기까지 시간을 세기 시작
    }
    return applied;                               // 실제로 깎인 양
}

void TrainingDummy::step(float dt_seconds) {
    if (!down() || dt_seconds <= 0.0f) return;     // 서 있으면 할 일 없음
    down_left_ = std::max(0.0f, down_left_ - dt_seconds);  // 남은 시간 줄이기
    if (down_left_ <= 0.0f) {                      // 다 됐으면
        hp_ = data_.max_hp;                        // HP를 가득 채워 다시 서요
    }
}

engine::RectF TrainingDummy::hurtbox() const {
    const auto& box = data_.hurtbox;                               // 발 기준 상자 설명
    return {feet_.x + box.x, feet_.y + box.y, box.w, box.h};       // 맵 위 위치로 옮겨요
}

}  // namespace wind::game
