// 이 파일은 무엇을 하나요?
// 주인공 마술이 나간 뒤: 빛 탄·별무리(돌기·조준·휘기)·달빛 칼날(한 번 베기).

#include "game/combat/hero_magic.hpp"

#include "game/combat/combat_random.hpp"
#include "game/combat/hitbox_placement.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/entities/facing.hpp"
#include "game/entities/player_motion.hpp"
#include "game/monsters/monster.hpp"

#include "wind/combat/damage.hpp"

#include <cmath>
#include <set>
#include <utility>

namespace wind::game {
namespace {

// 빛 탄이 나오는 곳: 발에서 앞으로 10픽셀, 화면 위로 14픽셀(가슴 높이). 이유는 docs/GAMEPLAY.md "Casting" 절.
constexpr float kSpellSpawnForward = 10.0f;
constexpr float kSpellLift = 14.0f;
// 별무리: 별 사이 떠나는 간격 (명세 "하나씩", 0.12초면 5개가 0.5초 안에 다 떠나요). docs/GAMEPLAY.md "Star swarm".
constexpr float kLaunchGap = 0.12f;
constexpr float kTwoPi = 6.28318531f;

// 네모 가운데
engine::Vec2 middle(const engine::RectF& r) {
    return {r.x + r.w * 0.5f, r.y + r.h * 0.5f};
}

// 바라보는 쪽의 각도 (라디안, 0 = 오른쪽, 화면 아래가 +)
float facing_angle(world::Direction facing) {
    const engine::Vec2 d = direction_vector(facing);
    return std::atan2(d.y, d.x);
}

}  // namespace

std::optional<engine::Vec2> nearest_enemy(engine::Vec2 from, float max_distance,
                                          std::span<const TrainingDummy> dummies,
                                          std::span<const Monster> monsters) {
    std::optional<engine::Vec2> best;
    float best_distance = max_distance;
    auto consider = [&](const engine::RectF& hurtbox) {
        const engine::Vec2 at = middle(hurtbox);
        const float distance = engine::length(at - from);
        if (distance <= best_distance) {
            best_distance = distance;
            best = at;
        }
    };
    for (const auto& dummy : dummies) {
        if (!dummy.defeated()) consider(dummy.hurtbox());  // 서 있는 허수아비만
    }
    for (const auto& monster : monsters) {
        if (!monster.defeated()) consider(monster.hurtbox());  // 쓰러지지 않은 몬스터만
    }
    return best;
}

engine::Vec2 HeroMagic::spell_hand(engine::Vec2 feet, world::Direction facing) {
    const engine::Vec2 ground = feet + direction_vector(facing) * kSpellSpawnForward;  // 발 조금 앞
    return {ground.x, ground.y - kSpellLift};                                         // 가슴 높이
}

float HeroMagic::magic_power() const {
    return combat::magic_attack_power(combat::kBaseMagicPower, intelligence_bonus_);  // 마법 공격력 (13a)
}

void HeroMagic::step(float dt_seconds, const PlayerMotion& player, const world::MapData& map,
                     std::span<TrainingDummy> dummies, std::span<Monster> monsters, CombatRandom& random) {
    hits_.clear();
    ended_.clear();
    slashes_.clear();
    release(player, dummies, monsters, random);  // 1) 이번 박자에 나간 마술
    beam_.step(dt_seconds, player, map, dummies, monsters, random, magic_power(), base_crit_chance_, hits_);  // 광선 (15d)
    const engine::Vec2 hub = player.mover().feet();
    for (auto& shot : projectiles_) {
        const auto hit_test = [&](const engine::RectF& rect) { return hit_first(shot, rect, dummies, monsters, random); };
        if (shot.orbiting()) {  // 2) 별무리: 주인공 둘레를 돌아요 (주인공을 따라와요)
            shot.orbit(hub, dt_seconds, hit_test);
        } else {
            const float homing = shot.shot().data.homing;
            if (homing > 0.0f) {  // 3) 가장 가까운 적 쪽으로: 막 떠난 별은 바로 조준, 날던 별은 조금씩 휘기
                const auto target = nearest_enemy(shot.center(), shot.shot().data.range, dummies, monsters);
                if (target && shot.travelled() <= 0.0f) shot.aim(*target - shot.center());
                if (target) shot.steer(*target, dt_seconds);
            }
            shot.step(dt_seconds, map, hit_test);  // 4) 날기 (벽·대상·거리)
        }
        if (!shot.alive()) ended_.push_back({shot.center(), shot.end()});  // 연출: 그 자리에서 반짝
    }
    std::erase_if(projectiles_, [](const Projectile& p) { return !p.alive(); });
}

void HeroMagic::release(const PlayerMotion& player, std::span<TrainingDummy> dummies, std::span<Monster> monsters,
                        CombatRandom& random) {
    const auto* spell = player.caster().spell();
    if (!player.spell_released() || spell == nullptr) return;
    if (spell->projectile) fire(*spell, player);
    if (spell->melee) slash(*spell, player, dummies, monsters, random);
}

// 빛 탄 count개. 별무리처럼 orbit_seconds가 있으면 주인공 둘레에 고르게 놓고, 별마다 조금씩 늦게 떠나요.
void HeroMagic::fire(const combat::Spell& spell, const PlayerMotion& player) {
    const auto& data = *spell.projectile;
    const world::Direction facing = player.cast_facing();
    for (int i = 0; i < data.count; ++i) {
        ProjectileShot shot;
        shot.direction = direction_vector(facing);                                     // 시전 방향
        shot.ground = player.mover().feet() + shot.direction * kSpellSpawnForward;     // 발 조금 앞
        shot.lift = kSpellLift;                                                        // 가슴 높이
        shot.data = data;                                                              // 빠르기·거리·크기·유도
        shot.attack_power = magic_power();
        shot.power = spell.power;                                                      // 마술 위력
        shot.crit_chance = base_crit_chance_;                                          // 마술도 기본 5%
        shot.knockback = spell.knockback;                                              // 타격감 (마술 데이터)
        shot.hitstop = spell.hitstop;
        shot.shake = spell.shake;
        shot.light = spell.light;  // 빛은 탄이 들고 다녀요 (연출, 15b)
        if (data.orbit_seconds > 0.0f) {
            shot.orbit_seconds = data.orbit_seconds + kLaunchGap * static_cast<float>(i);  // 하나씩 떠나요
            shot.orbit_angle = facing_angle(facing) + kTwoPi * static_cast<float>(i) / static_cast<float>(data.count);
        }
        projectiles_.emplace_back(shot);
    }
}

// 달빛 칼날: 시전 방향 앞 상자에 닿은 대상을 한 번씩 (허수아비·몬스터 모두)
void HeroMagic::slash(const combat::Spell& spell, const PlayerMotion& player, std::span<TrainingDummy> dummies,
                      std::span<Monster> monsters, CombatRandom& random) {
    const world::Direction facing = player.cast_facing();
    Swing swing;
    swing.hitbox = place_hitbox(*spell.melee, player.mover().feet(), facing);
    swing.attack_power = magic_power();
    swing.power = spell.power;
    swing.crit_chance = base_crit_chance_;
    swing.push_direction = direction_vector(facing);
    swing.knockback = spell.knockback;
    swing.hitstop = spell.hitstop;
    swing.shake = spell.shake;
    std::set<TargetId> struck;  // 이번 베기에서 맞힌 대상 (한 번씩만)
    swing.register_hit = [&struck](TargetId id) { return struck.insert(id).second; };
    for (const auto& hit : resolve_swing(swing, dummies, &random)) hits_.push_back(hit);
    for (const auto& hit : resolve_swing(swing, monsters, &random)) hits_.push_back(hit);
    slashes_.push_back({swing.hitbox, facing, spell.light});
}

bool HeroMagic::hit_first(const Projectile& shot, const engine::RectF& rect, std::span<TrainingDummy> dummies,
                          std::span<Monster> monsters, CombatRandom& random) {
    bool used = false;  // 이 걸음에서 이미 하나 맞혔나
    Swing swing;
    swing.hitbox = rect;
    swing.attack_power = shot.shot().attack_power;
    swing.power = shot.shot().power;
    swing.crit_chance = shot.shot().crit_chance;
    swing.push_direction = shot.shot().direction;  // 날아가던 쪽으로 밀어요
    swing.knockback = shot.shot().knockback;
    swing.hitstop = shot.shot().hitstop;
    swing.shake = shot.shot().shake;
    swing.register_hit = [&used](TargetId) { return !std::exchange(used, true); };  // 첫 대상만
    auto hits = resolve_swing(swing, dummies, &random);                  // 허수아비 먼저
    if (hits.empty()) hits = resolve_swing(swing, monsters, &random);    // 없으면 몬스터 (11d)
    hits_.insert(hits_.end(), hits.begin(), hits.end());
    return !hits.empty();
}

}  // namespace wind::game
