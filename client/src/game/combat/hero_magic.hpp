#pragma once

// 이 파일은 무엇을 하나요?
// 주인공 마술의 "나간 뒤" 담당이에요. (명세 8.6, 9.2, 15c) FieldCombat이 300줄을 넘지 않게 여기로 모았어요.
// 시전이 끝난 박자(spell_released)에 마술 동작에 따라:
//   - 빛 탄(별빛 조각·큰 별빛 조각): 손끝에서 시전 방향으로 하나
//   - 별무리: 작은 별 count개가 주인공 둘레를 돌다가(orbit_seconds, 별마다 0.12초씩 늦게) 하나씩 떠나요.
//     떠날 때 range 안에 적이 있으면 가장 가까운 적 쪽으로, 없으면 시전 방향으로 날아가요.
//     날아가는 동안 가장 가까운 적 쪽으로 휘어요(homing). 도는 동안 닿은 적도 맞히고 그 별은 사라져요.
//   - 달빛 칼날(근접): 그 박자에 앞쪽 상자(melee)에 닿은 대상을 한 번씩 베요.
//   - 혜성 광선(15d): 쏘는 동안 HeroBeam이 0.1초마다 띠 안의 적을 모두 맞혀요.
// 피해 = 마법 공격력(지능) × 위력, 그다음은 칼과 같은 규칙(resolve_swing)이에요.
// 적 = 서 있는 허수아비와 쓰러지지 않은 몬스터. 연출용으로 이번 박자의 맞은 일·사라진 탄·벤 자리를 남겨요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/hero_beam.hpp"
#include "game/combat/hit_resolution.hpp"
#include "game/combat/projectile.hpp"

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

// 달빛 칼날이 벤 자리 (연출: 푸른 칼자국, 15c)
struct SpellSlashEvent {
    engine::RectF box;                                 // 벤 상자 (맵 위)
    world::Direction facing = world::Direction::Down;  // 벤 방향
    std::optional<combat::SpellLight> light;           // 칼날 빛
};

class HeroMagic {
public:
    explicit HeroMagic(float base_crit_chance) : base_crit_chance_(base_crit_chance) {}

    // 지능 보너스 (마법 공격력 = 12 × (1 + 이 값), 13a)
    void set_intelligence_bonus(float bonus) { intelligence_bonus_ = bonus; }

    // 한 박자: 나간 마술 만들기 → 별 돌리기·떠나기 → 휘기·날리기 → 맞은 일 모으기
    void step(float dt_seconds, const PlayerMotion& player, const world::MapData& map,
              std::span<TrainingDummy> dummies, std::span<Monster> monsters, CombatRandom& random);

    [[nodiscard]] const std::vector<Projectile>& projectiles() const { return projectiles_; }
    // 이번 박자에 맞힌 일, 사라진 탄, 벤 자리 (다음 step에서 지워져요)
    [[nodiscard]] const std::vector<HitEvent>& hits() const { return hits_; }
    [[nodiscard]] const std::vector<ProjectileEndEvent>& ended() const { return ended_; }
    [[nodiscard]] const std::vector<SpellSlashEvent>& slashes() const { return slashes_; }
    // 혜성 광선 (켜져 있나, 모양, 15d)
    [[nodiscard]] const HeroBeam& beam() const { return beam_; }

    // 손끝 (빛 탄이 나오는 곳, 몸 높이): 발에서 앞으로 10픽셀, 위로 14픽셀
    [[nodiscard]] static engine::Vec2 spell_hand(engine::Vec2 feet, world::Direction facing);

private:
    void release(const PlayerMotion& player, std::span<TrainingDummy> dummies, std::span<Monster> monsters,
                 CombatRandom& random);
    void fire(const combat::Spell& spell, const PlayerMotion& player);
    void slash(const combat::Spell& spell, const PlayerMotion& player, std::span<TrainingDummy> dummies,
               std::span<Monster> monsters, CombatRandom& random);
    // 탄 하나가 이 네모에서 처음 닿은 대상 하나를 맞혀요 (허수아비 먼저, 없으면 몬스터)
    bool hit_first(const Projectile& shot, const engine::RectF& rect, std::span<TrainingDummy> dummies,
                   std::span<Monster> monsters, CombatRandom& random);
    [[nodiscard]] float magic_power() const;

    float base_crit_chance_ = 0.0f;
    float intelligence_bonus_ = 0.0f;
    std::vector<Projectile> projectiles_;     // 돌거나 날고 있는 빛 탄·별
    std::vector<HitEvent> hits_;              // 이번 박자
    std::vector<ProjectileEndEvent> ended_;   // 이번 박자
    std::vector<SpellSlashEvent> slashes_;    // 이번 박자
    HeroBeam beam_;                           // 혜성 광선 (15d)
};

// 가장 가까운 적의 몸 가운데 (from에서 max_distance 안, 없으면 nullopt). 별무리 조준·휘기에 써요 (15c)
[[nodiscard]] std::optional<engine::Vec2> nearest_enemy(engine::Vec2 from, float max_distance,
                                                        std::span<const TrainingDummy> dummies,
                                                        std::span<const Monster> monsters);

}  // namespace wind::game
