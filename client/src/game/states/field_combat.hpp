#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 싸움 담당이에요. (9단계 9c, 9e)
// - 맵에 놓인 훈련용 허수아비들과, 날아가는 빛 탄들을 가지고 있어요.
// - 정해진 박자마다:
//   1) 주인공 칼이 판정 중이면 맞았는지 보고(hit_resolution) 피해를 주고 밀어내요.
//      치명타는 시드가 있는 싸움 주사위로 굴려요 (기본 5%, 10a).
//   2) 주인공 시전이 이번 박자에 끝났으면 빛 탄을 하나 만들어요.
//   3) 빛 탄을 날려요. 허수아비에 맞으면 피해를 주고 사라지고, 벽·거리 끝에서도 사라져요.
//   4) 허수아비 시간을 흘려요 (쓰러졌다 다시 서기).
// - 이번 박자에 맞은 일, 사라진 빛 탄, 마지막으로 맞은 일을 기억해요 (연출과 F3 정보판이 봐요).
// 게임 규칙이라 SDL을 몰라요.

#include "game/combat/hit_resolution.hpp"
#include "game/combat/projectile.hpp"
#include "game/combat/training_dummy.hpp"
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
    // seed: 치명타 주사위 씨앗 (같으면 같은 결과), base_crit_chance: 기본 치명타 확률
    explicit FieldCombat(std::vector<TrainingDummy> dummies, std::uint64_t seed = 1,
                         float base_crit_chance = kBaseCritChance);

    // 한 박자: 칼 판정 -> 빛 탄 만들기 -> 빛 탄 날리기 -> 허수아비 시간 흘리기
    void step(double dt_seconds, PlayerMotion& player, const world::MapData& map);

    [[nodiscard]] const std::vector<TrainingDummy>& dummies() const { return dummies_; }
    // 날고 있는 빛 탄들
    [[nodiscard]] const std::vector<Projectile>& projectiles() const { return projectiles_; }
    // 이번 박자에 맞은 일들 (다음 박자에 지워져요)
    [[nodiscard]] const std::vector<HitEvent>& hits() const { return hits_; }
    // 이번 박자에 사라진 빛 탄들 (다음 박자에 지워져요)
    [[nodiscard]] const std::vector<ProjectileEndEvent>& projectile_ends() const { return ended_; }
    // 마지막으로 맞은 일 (F3 정보판용)
    [[nodiscard]] const std::optional<HitEvent>& last_hit() const { return last_hit_; }
    // F3 정보판의 싸움 줄: "dummies 2  shards 1  last hit 14 -> #1 86/100 (down)"
    [[nodiscard]] std::string debug_text() const;

    // 빛 탄이 나오는 손끝 (몸 높이). 시전 연출도 같은 자리에서 빛나요.
    [[nodiscard]] static engine::Vec2 spell_hand(engine::Vec2 feet, world::Direction facing);

private:
    void resolve_sword(PlayerMotion& player);
    void spawn_spell(const PlayerMotion& player);
    void step_projectiles(float dt_seconds, const world::MapData& map);
    // 맞은 일을 이번 박자 목록과 "마지막으로 맞은 일"에 적어요
    void record_hits(const std::vector<HitEvent>& hits);

    std::vector<TrainingDummy> dummies_;      // 맵의 허수아비들
    std::vector<Projectile> projectiles_;     // 날고 있는 빛 탄들
    std::vector<HitEvent> hits_;              // 이번 박자에 맞은 일
    std::vector<ProjectileEndEvent> ended_;   // 이번 박자에 사라진 빛 탄
    std::optional<HitEvent> last_hit_;        // 마지막으로 맞은 일
    CombatRandom random_;                     // 치명타 주사위 (시드가 있어 재현 가능)
    float base_crit_chance_;                  // 기본 치명타 확률
};

}  // namespace wind::game
