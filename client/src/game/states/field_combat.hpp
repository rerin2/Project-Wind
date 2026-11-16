#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 싸움 담당이에요. (9단계 9c)
// - 맵에 놓인 훈련용 허수아비들을 가지고 있어요.
// - 정해진 박자마다: 주인공 칼이 판정 중이면 맞았는지 보고(hit_resolution), 허수아비 시간을 흘려요.
// - 이번 박자에 맞은 일과 마지막으로 맞은 일을 기억해요 (연출과 F3 정보판이 봐요).
// 게임 규칙이라 SDL을 몰라요.

#include "game/combat/hit_resolution.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/entities/player_motion.hpp"

#include <optional>
#include <string>
#include <vector>

namespace wind::game {

class FieldCombat {
public:
    explicit FieldCombat(std::vector<TrainingDummy> dummies);

    // 한 박자: 칼 판정 -> 피해 적용 -> 허수아비 시간 흘리기
    void step(double dt_seconds, PlayerMotion& player);

    [[nodiscard]] const std::vector<TrainingDummy>& dummies() const { return dummies_; }
    // 이번 박자에 맞은 일들 (다음 박자에 지워져요)
    [[nodiscard]] const std::vector<HitEvent>& hits() const { return hits_; }
    // 마지막으로 맞은 일 (F3 정보판용)
    [[nodiscard]] const std::optional<HitEvent>& last_hit() const { return last_hit_; }
    // F3 정보판의 싸움 줄: "dummies 2  last hit 14 -> #1 86/100 (down)"
    [[nodiscard]] std::string debug_text() const;

private:
    std::vector<TrainingDummy> dummies_;  // 맵의 허수아비들
    std::vector<HitEvent> hits_;          // 이번 박자에 맞은 일
    std::optional<HitEvent> last_hit_;    // 마지막으로 맞은 일
};

}  // namespace wind::game
