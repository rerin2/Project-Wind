#pragma once

// 이 파일은 무엇을 하나요?
// 혜성 광선(15d, 명세 8.6)의 "맞히기" 담당이에요. HeroMagic이 가지고 박자마다 불러요.
// - 주인공이 광선을 쏘는 중(SpellCaster::beaming)이면 손끝에서 시전 방향으로 곧은 띠를 놓아요.
// - 띠는 벽(못 가는 지형)에서 잘리고(개발자와 정함), 길이는 beam.length까지예요.
// - tick_seconds(0.1초)마다 띠 안의 모든 적을 한 번씩 맞혀요(뚫고 지나감, 개발자와 정함).
//   쏘기 시작한 박자에 첫 번째로 맞혀요. 위력은 "초당"이라 한 번에 power × tick_seconds예요.
// - 버튼을 떼거나, MP가 떨어지거나, 구르거나, 맞으면 PlayerMotion이 광선을 끝내고, 여기서도 꺼져요.
// 그리기는 BeamView가 이 담당의 모양(방향·길이·굵기·막힘)을 보고 해요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/hit_resolution.hpp"

#include "wind/combat/spell.hpp"
#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

#include <optional>
#include <span>
#include <vector>

namespace wind::game {

class CombatRandom;
class Monster;
class PlayerMotion;
class TrainingDummy;

class HeroBeam {
public:
    // 한 박자: 광선이 켜져 있으면 모양을 다시 재고(주인공을 따라와요), 때가 되면 띠 안의 적을 맞혀요.
    // attack_power: 마법 공격력, crit_chance: 치명타 확률. 맞힌 일은 hits 끝에 붙여요.
    void step(float dt_seconds, const PlayerMotion& player, const world::MapData& map,
              std::span<TrainingDummy> dummies, std::span<Monster> monsters, CombatRandom& random,
              float attack_power, float crit_chance, std::vector<HitEvent>& hits);

    [[nodiscard]] bool active() const { return active_; }
    [[nodiscard]] world::Direction facing() const { return facing_; }
    // 벽에서 잘린 뒤의 길이 (픽셀, 0이면 손끝이 이미 벽 안)
    [[nodiscard]] float length() const { return length_; }
    [[nodiscard]] float width() const { return width_; }
    // 벽에 막혀 짧아졌나요? (연출: 끝에서 불꽃이 튀어요)
    [[nodiscard]] bool blocked() const { return blocked_; }
    // 맞는 띠 (맵 위, 몸 높이). 꺼져 있으면 빈 네모
    [[nodiscard]] const engine::RectF& rect() const { return rect_; }
    // 광선 빛 (연출, 마술 데이터 light)
    [[nodiscard]] const std::optional<combat::SpellLight>& light() const { return light_; }

private:
    // 띠 안의 모든 적을 한 번씩 맞혀요 (한 번 맞히기 = tick)
    void strike(const combat::Spell& spell, std::span<TrainingDummy> dummies, std::span<Monster> monsters,
                CombatRandom& random, float attack_power, float crit_chance, std::vector<HitEvent>& hits) const;

    bool active_ = false;
    world::Direction facing_ = world::Direction::Down;
    float length_ = 0.0f;
    float width_ = 0.0f;
    bool blocked_ = false;
    engine::RectF rect_{};
    std::optional<combat::SpellLight> light_;
    float tick_left_ = 0.0f;  // 다음 맞히기까지 남은 시간 (0 이하면 이번 박자에 맞혀요)
};

// 손끝(몸 높이)에서 facing 쪽으로 length만큼, 굵기 width인 띠 (맵 위 네모)
[[nodiscard]] engine::RectF beam_rect(engine::Vec2 hand, world::Direction facing, float length, float width);
// 땅 위 start에서 direction(길이 1) 쪽으로 가다 처음 벽을 만나는 거리 (max_length까지, 픽셀)
[[nodiscard]] float beam_reach(const world::MapData& map, engine::Vec2 start, engine::Vec2 direction,
                               float max_length);

}  // namespace wind::game
