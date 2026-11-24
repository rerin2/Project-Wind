// 이 파일은 무엇을 하나요?
// 혜성 광선의 모양(벽에서 잘린 띠)을 재고, 0.1초마다 띠 안의 적을 모두 맞히는 곳이에요.
// 값의 이유는 docs/GAMEPLAY.md "Comet beam" 절에 있어요.

#include "game/combat/hero_beam.hpp"

#include "game/combat/combat_random.hpp"
#include "game/combat/hero_magic.hpp"
#include "game/combat/projectile.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/entities/facing.hpp"
#include "game/entities/player_motion.hpp"
#include "game/monsters/monster.hpp"

#include <algorithm>
#include <set>

namespace wind::game {
namespace {

constexpr float kReachStep = 2.0f;       // 벽 찾기 걸음 (픽셀): 작을수록 벽에 딱 붙어 멈춰요
constexpr float kTimeEpsilon = 1e-4f;    // 1/60초를 더할 때 생기는 아주 작은 오차 여유
constexpr int kMaxStrikesPerStep = 4;    // 한 박자에 맞히기 최대 (아주 긴 박자가 와도 멈추지 않게)
constexpr float kSpawnForward = 10.0f;   // 손끝 땅 위치: 발에서 앞으로 (HeroMagic 빛 탄과 같아요)

}  // namespace

engine::RectF beam_rect(engine::Vec2 hand, world::Direction facing, float length, float width) {
    const float half = width * 0.5f;
    switch (facing) {
        case world::Direction::Right: return {hand.x, hand.y - half, length, width};
        case world::Direction::Left: return {hand.x - length, hand.y - half, length, width};
        case world::Direction::Down: return {hand.x - half, hand.y, width, length};
        case world::Direction::Up: return {hand.x - half, hand.y - length, width, length};
    }
    return {hand.x, hand.y - half, length, width};
}

float beam_reach(const world::MapData& map, engine::Vec2 start, engine::Vec2 direction, float max_length) {
    // 작은 걸음으로 앞으로 가며, 처음 벽인 점에서 멈춰요 (빛 탄과 같은 벽 규칙: 허수아비 칸은 벽이 아님)
    for (float d = 0.0f; d < max_length; d += kReachStep) {
        if (projectile_blocked(map, start + direction * d)) return d;
    }
    return max_length;  // 끝까지 벽이 없어요
}

void HeroBeam::step(float dt_seconds, const PlayerMotion& player, const world::MapData& map,
                    std::span<TrainingDummy> dummies, std::span<Monster> monsters, CombatRandom& random,
                    float attack_power, float crit_chance, std::vector<HitEvent>& hits) {
    const combat::Spell* spell = player.caster().spell();
    if (!player.caster().beaming() || spell == nullptr || !spell->beam) {
        active_ = false;  // 광선이 꺼졌어요: 다음에 켜지면 바로 첫 번째로 맞혀요
        rect_ = {};
        return;
    }
    const combat::BeamData& beam = *spell->beam;
    if (!active_) {
        active_ = true;
        tick_left_ = 0.0f;  // 켜진 박자에 첫 번째로 맞혀요
    }
    // 모양: 주인공을 따라와요. 땅 위로 벽을 찾고, 몸 높이 손끝에서 띠를 놓아요
    facing_ = player.cast_facing();
    const engine::Vec2 direction = direction_vector(facing_);
    const engine::Vec2 ground = player.mover().feet() + direction * kSpawnForward;
    length_ = std::max(0.0f, beam_reach(map, ground, direction, beam.length));
    blocked_ = length_ < beam.length;
    width_ = beam.width;
    rect_ = beam_rect(HeroMagic::spell_hand(player.mover().feet(), facing_), facing_, length_, width_);
    light_ = spell->light;
    // 맞히기: tick_seconds마다 (한 박자에 너무 많이는 안 돼요)
    tick_left_ -= std::max(0.0f, dt_seconds);
    for (int i = 0; i < kMaxStrikesPerStep && tick_left_ <= kTimeEpsilon; ++i) {
        strike(*spell, dummies, monsters, random, attack_power, crit_chance, hits);
        tick_left_ += beam.tick_seconds;  // 로더가 0보다 큰 값만 받아요
    }
    tick_left_ = std::max(tick_left_, 0.0f);  // 너무 밀린 맞히기는 버려요
}

void HeroBeam::strike(const combat::Spell& spell, std::span<TrainingDummy> dummies, std::span<Monster> monsters,
                      CombatRandom& random, float attack_power, float crit_chance,
                      std::vector<HitEvent>& hits) const {
    if (length_ <= 0.0f) return;  // 손끝이 벽 안이면 아무것도 안 맞아요
    Swing swing;
    swing.hitbox = rect_;
    swing.attack_power = attack_power;
    swing.power = spell.power * spell.beam->tick_seconds;  // "초당 3.0" → 한 번에 0.3
    swing.crit_chance = crit_chance;
    swing.push_direction = direction_vector(facing_);     // 광선 쪽으로 살짝 밀어요
    swing.knockback = spell.knockback;
    swing.hitstop = spell.hitstop;
    swing.shake = spell.shake;
    std::set<TargetId> struck;  // 이번 맞히기에서 한 대상은 한 번만 (뚫고 지나가며 모두)
    swing.register_hit = [&struck](TargetId id) { return struck.insert(id).second; };
    for (const auto& hit : resolve_swing(swing, dummies, &random)) hits.push_back(hit);
    for (const auto& hit : resolve_swing(swing, monsters, &random)) hits.push_back(hit);
}

}  // namespace wind::game
