#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 싸움 담당이에요. (9단계 9c, 9e, 11a)
// - 맵에 놓인 훈련용 허수아비들, 몬스터들, 날아가는 빛 탄들을 가지고 있어요.
// - 정해진 박자마다:
//   1) 주인공 칼이 판정 중이면 허수아비·몬스터(11d)에 맞았는지 보고(hit_resolution) 피해를 주고 밀어내요.
//      치명타는 시드가 있는 싸움 주사위로 굴려요 (기본 5%, 10a).
//   2) 주인공 시전이 이번 박자에 끝났으면 빛 탄을 하나 만들어요.
//   3) 빛 탄을 날려요. 허수아비·몬스터에 맞으면 피해를 주고 사라지고, 벽·거리 끝에서도 사라져요.
//   4) 허수아비 시간을 흘려요 (쓰러졌다 다시 서기).
//   5) 몬스터를 움직여요 (11a·11b). 몬스터 주사위는 치명타 주사위와 따로 굴려요.
//   6) 덮치는 몬스터의 판정이 주인공 맞는 상자에 닿으면 주인공이 맞아요 (덮치기 한 번에 한 번, 무적이면 안 맞음, 11c).
//   7) 겹친 몸(주인공·몬스터, 몬스터끼리)을 부드럽게 떼어 놓아요 (명세 7.3, 11c).
// - 주인공이 쓰러지면(12b) 몬스터에게는 주인공이 "없음"으로 보여서 포기하고 제자리로 돌아가요.
//   쓰러진 몸은 밀어내기에서도 빠져요.
// - 이번 박자에 맞은 일, 사라진 빛 탄, 마지막으로 맞은 일을 기억해요 (연출과 F3 정보판이 봐요).
// 게임 규칙이라 SDL을 몰라요.

#include "game/combat/hero_hit_event.hpp"
#include "game/combat/hit_resolution.hpp"
#include "game/combat/projectile.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/monsters/monster.hpp"
#include "game/monsters/monster_vanish_event.hpp"
#include "game/entities/player_motion.hpp"

#include "wind/world/map.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace wind::game {

// 기본 치명타 확률 (10a, 개발자와 정함). 무기 crit_bonus를 더해요. 이유는 docs/GAMEPLAY.md "Critical hits".
inline constexpr float kBaseCritChance = 0.05f;

class FieldCombat {
public:
    // seed: 치명타·몬스터 주사위 씨앗 (같으면 같은 결과), base_crit_chance: 기본 치명타 확률
    // monsters: 맵의 몬스터들 (11a)
    explicit FieldCombat(std::vector<TrainingDummy> dummies, std::uint64_t seed = 1,
                         float base_crit_chance = kBaseCritChance, std::vector<Monster> monsters = {});

    // 한 박자: 칼 판정 -> 빛 탄 만들기 -> 빛 탄 날리기 -> 허수아비 시간 흘리기 -> 몬스터 움직이기
    //         -> 몬스터 덮치기가 주인공을 맞혔나 -> 겹친 몸 떼어 놓기
    void step(double dt_seconds, PlayerMotion& player, const world::MapData& map);

    [[nodiscard]] const std::vector<TrainingDummy>& dummies() const { return dummies_; }
    [[nodiscard]] const std::vector<Monster>& monsters() const { return monsters_; }
    // 날고 있는 빛 탄들
    [[nodiscard]] const std::vector<Projectile>& projectiles() const { return projectiles_; }
    // 이번 박자에 맞은 일들 (다음 박자에 지워져요)
    [[nodiscard]] const std::vector<HitEvent>& hits() const { return hits_; }
    // 이번 박자에 사라진 빛 탄들 (다음 박자에 지워져요)
    [[nodiscard]] const std::vector<ProjectileEndEvent>& projectile_ends() const { return ended_; }
    // 이번 박자에 주인공이 맞은 일 (다음 박자에 지워져요, 11c)
    [[nodiscard]] const std::vector<HeroHitEvent>& hero_hits() const { return hero_hits_; }
    // 이번 박자에 다 사라진(쓰러진 뒤 0.5초) 몬스터들 (다음 박자에 지워져요, 11d)
    [[nodiscard]] const std::vector<MonsterVanishEvent>& vanished() const { return vanished_; }
    // 이번 박자에 맞은 일(허수아비·주인공) 중 가장 긴 멈칫 시간 (아무도 안 맞았으면 0). 히트스톱에 넘겨요.
    [[nodiscard]] float strongest_hitstop() const;
    // 이번 박자에 주인공이 마지막 한 방을 맞고 쓰러졌나요? (12b: 슬로모션 시작)
    [[nodiscard]] bool hero_fell() const;
    // 마지막으로 맞은 일 (F3 정보판용)
    [[nodiscard]] const std::optional<HitEvent>& last_hit() const { return last_hit_; }
    // F3 정보판의 싸움 줄: "dummies 2  mobs 2 (chase)  shards 1  last hit 14 -> #1 86/100 (down)"
    [[nodiscard]] std::string debug_text() const;

    // 빛 탄이 나오는 손끝 (몸 높이). 시전 연출도 같은 자리에서 빛나요.
    [[nodiscard]] static engine::Vec2 spell_hand(engine::Vec2 feet, world::Direction facing);

private:
    void resolve_sword(PlayerMotion& player);
    void spawn_spell(const PlayerMotion& player);
    void step_projectiles(float dt_seconds, const world::MapData& map);
    // 맞은 일을 이번 박자 목록과 "마지막으로 맞은 일"에 적어요
    void record_hits(const std::vector<HitEvent>& hits);
    // 덮치는 몬스터가 주인공을 맞혔나 보고, 맞았으면 피해·경직·넉백을 줘요 (11c)
    void resolve_monster_attacks(PlayerMotion& player);
    // 겹친 몸을 떼어 놓아요: 주인공 ↔ 몬스터, 몬스터 ↔ 몬스터 (11c)
    void separate(PlayerMotion& player, const world::MapData& map, float dt_seconds);

    std::vector<TrainingDummy> dummies_;      // 맵의 허수아비들
    std::vector<Monster> monsters_;           // 맵의 몬스터들 (11a)
    std::vector<Projectile> projectiles_;     // 날고 있는 빛 탄들
    std::vector<HitEvent> hits_;              // 이번 박자에 맞은 일
    std::vector<ProjectileEndEvent> ended_;   // 이번 박자에 사라진 빛 탄
    std::vector<HeroHitEvent> hero_hits_;     // 이번 박자에 주인공이 맞은 일 (11c)
    std::vector<MonsterVanishEvent> vanished_;  // 이번 박자에 다 사라진 몬스터 (11d)
    std::optional<HitEvent> last_hit_;        // 마지막으로 맞은 일
    CombatRandom random_;                     // 치명타 주사위 (시드가 있어 재현 가능)
    CombatRandom monster_random_;             // 몬스터 행동 주사위 (치명타와 따로: 칼질이 걷는 길을 바꾸지 않게)
    float base_crit_chance_;                  // 기본 치명타 확률
};

}  // namespace wind::game
