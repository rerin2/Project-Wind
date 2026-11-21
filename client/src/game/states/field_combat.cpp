// 이 파일은 무엇을 하나요?
// 필드의 칼 판정, 빛 탄 만들기·날리기, 허수아비 시간, 몬스터 움직임을 박자마다 진행하는 곳이에요.

#include "game/states/field_combat.hpp"

#include "game/entities/facing.hpp"
#include "game/physics/body_separation.hpp"

#include "wind/log.hpp"

#include <algorithm>
#include <string>
#include <utility>

namespace wind::game {
namespace {

// 힘 보너스 (명세 8.2). 능력치가 생기는 13단계 전까지는 시작 힘 10의 보너스 0이에요.
constexpr float kStrengthBonus = 0.0f;
// 기본 마력과 지능 보너스 (명세 8.2). 13단계 전까지는 시작 지능 10의 보너스 0이라 마법 공격력 12예요.
constexpr float kBaseMagic = 12.0f;
constexpr float kIntelligenceBonus = 0.0f;
// 빛 탄이 나오는 곳: 발에서 앞으로 10픽셀, 화면 위로 14픽셀(가슴 높이)
// 이유는 docs/GAMEPLAY.md "Casting" 절에 있어요.
constexpr float kSpellSpawnForward = 10.0f;
constexpr float kSpellLift = 14.0f;
// 몬스터 주사위 씨앗 = 싸움 씨앗 XOR 이 값. 같은 씨앗에서 치명타와 다른 줄의 수가 나와요 (0이 아닌 아무 수나 괜찮아요).
constexpr std::uint64_t kMonsterStream = 0xD1B54A32D192ED03ULL;
// 주인공 방어력 (명세 8.2·10.3). 장비가 생기는 15단계 전까지는 0이에요.
constexpr float kHeroDefense = 0.0f;
// 몸끼리 밀어내기 (명세 7.3, 11c). 이유는 docs/GAMEPLAY.md "Bodies pushing apart" 절에 있어요.
constexpr float kSeparationSpeed = 120.0f;  // 한 쌍을 1초에 떼어 놓는 최대 거리 (픽셀/초, 힘 제한)
constexpr float kHeroPushWeight = 1.0f;     // 주인공이 밀리는 몫 (몬스터의 1/3)
constexpr float kMonsterPushWeight = 3.0f;  // 몬스터가 밀리는 몫 (슬라임은 가벼워요)

}  // namespace

FieldCombat::FieldCombat(std::vector<TrainingDummy> dummies, std::uint64_t seed, float base_crit_chance,
                         std::vector<Monster> monsters)
    : dummies_(std::move(dummies)),
      monsters_(std::move(monsters)),
      random_(seed),
      monster_random_(seed ^ kMonsterStream),
      base_crit_chance_(base_crit_chance) {
    // 몬스터의 맞는 대상 번호는 허수아비 다음부터 (같은 휘두르기에 두 번 맞지 않게 서로 달라야 해요, 11d)
    for (std::size_t i = 0; i < monsters_.size(); ++i) {
        monsters_[i].set_id(static_cast<TargetId>(dummies_.size() + i + 1));
    }
}

void FieldCombat::step(double dt_seconds, PlayerMotion& player, const world::MapData& map) {
    hits_.clear();   // 지난 박자 기록 지우기
    ended_.clear();
    hero_hits_.clear();
    vanished_.clear();
    const auto dt = static_cast<float>(dt_seconds);
    resolve_sword(player);         // 1) 칼
    spawn_spell(player);           // 2) 시전이 끝났으면 빛 탄 하나
    step_projectiles(dt, map);     // 3) 빛 탄 날리기
    for (auto& dummy : dummies_) { // 4) 쓰러진 허수아비가 다시 서기까지 시간 세기
        dummy.step(dt);
    }
    // 몬스터가 보는 주인공 발 위치. 쓰러졌으면 "없음"이라 쫓던 몬스터는 포기하고 돌아가요 (12b)
    const std::optional<engine::Vec2> hero =
        player.dead() ? std::nullopt : std::optional<engine::Vec2>{player.mover().feet()};
    for (std::size_t i = 0; i < monsters_.size(); ++i) {  // 5) 몬스터: 쉬기·돌아다니기·쫓기·덮치기·쓰러지기
        auto& monster = monsters_[i];
        const bool was_gone = monster.gone();  // 이번 박자에 다 사라졌는지 보려고 (11d)
        monster.step(dt, map, monster_random_, hero);  // 주인공 발 위치를 보고 알아채요 (11b)
        if (!was_gone && monster.gone()) vanished_.push_back({i, monster.feet()});  // 사라짐 연출용 (11d)
    }
    resolve_monster_attacks(player);  // 6) 덮치기가 주인공에게 닿았나 (11c)
    separate(player, map, dt);         // 7) 겹친 몸 떼어 놓기 (11c)
}

// 칼이 판정 중이면 허수아비에 맞았는지 보고 피해를 줘요 (9c).
void FieldCombat::resolve_sword(PlayerMotion& player) {
    const auto hitbox = player.attack_hitbox();                 // 판정 중이 아니면 비어 있어요
    const auto* attack = player.attack().current();             // 지금 휘두르는 공격
    if (!hitbox || attack == nullptr || (dummies_.empty() && monsters_.empty())) {
        return;
    }
    const auto& weapon = player.attack().weapon();              // 든 무기
    Swing swing;                                                // 이번 칼 한 번
    swing.hitbox = *hitbox;                                     // 닿는 곳
    swing.attack_power = combat::physical_attack_power(weapon.base_power, weapon.strength_scaling,
                                                       kStrengthBonus);  // 공격력
    swing.power = attack->power;                                // 콤보마다 다른 위력
    swing.crit_chance = base_crit_chance_ + weapon.crit_bonus;  // 기본 5% + 무기 보너스
    swing.push_direction = direction_vector(player.mover().facing());  // 바라보는 쪽으로 밀어요
    swing.knockback = attack->knockback;                        // 타격감은 공격 데이터 값 그대로
    swing.hitstop = attack->hitstop;
    swing.shake = attack->shake;
    swing.register_hit = [&player](TargetId id) { return player.register_hit(id); };  // 한 번만 맞게
    record_hits(resolve_swing(swing, dummies_, &random_));      // 맞았는지 보고 피해 주기
    record_hits(resolve_swing(swing, monsters_, &random_));     // 몬스터도 같은 규칙으로 (11d)
}

// 주인공 시전이 이번 박자에 끝났으면, 시전 방향으로 빛 탄을 하나 만들어요.
void FieldCombat::spawn_spell(const PlayerMotion& player) {
    const auto* spell = player.caster().spell();
    if (!player.spell_released() || spell == nullptr || !spell->projectile) {
        return;
    }
    ProjectileShot shot;
    shot.direction = direction_vector(player.cast_facing());                      // 시전 방향
    shot.ground = player.mover().feet() + shot.direction * kSpellSpawnForward;     // 발 조금 앞
    shot.lift = kSpellLift;                                                        // 가슴 높이
    shot.data = *spell->projectile;                                                // 빠르기·거리·크기
    shot.attack_power = combat::magic_attack_power(kBaseMagic, kIntelligenceBonus);  // 마법 공격력
    shot.power = spell->power;                                                     // 마술 위력
    shot.crit_chance = base_crit_chance_;                                          // 마술도 기본 5%
    shot.knockback = spell->knockback;                                             // 타격감 (마술 데이터)
    shot.hitstop = spell->hitstop;
    shot.shake = spell->shake;
    projectiles_.emplace_back(shot);
}

// 빛 탄을 날리고, 사라진 것은 기록한 뒤 목록에서 빼요.
void FieldCombat::step_projectiles(float dt_seconds, const world::MapData& map) {
    for (auto& projectile : projectiles_) {
        // 탄 네모가 허수아비에 닿았는지 묻는 함수: 처음 닿은 하나만 맞아요
        const Projectile::HitTest hit_test = [this, &projectile](const engine::RectF& rect) {
            bool used = false;                               // 이 걸음에서 이미 하나 맞혔나
            Swing swing;
            swing.hitbox = rect;
            swing.attack_power = projectile.shot().attack_power;
            swing.power = projectile.shot().power;
            swing.crit_chance = projectile.shot().crit_chance;
            swing.push_direction = projectile.shot().direction;    // 날아가던 쪽으로 밀어요
            swing.knockback = projectile.shot().knockback;
            swing.hitstop = projectile.shot().hitstop;
            swing.shake = projectile.shot().shake;
            swing.register_hit = [&used](TargetId) { return !std::exchange(used, true); };  // 첫 대상만
            auto hits = resolve_swing(swing, dummies_, &random_);       // 허수아비 먼저
            if (hits.empty()) hits = resolve_swing(swing, monsters_, &random_);  // 아무도 안 맞았으면 몬스터 (11d)
            record_hits(hits);
            return !hits.empty();
        };
        projectile.step(dt_seconds, map, hit_test);
        if (!projectile.alive()) {
            ended_.push_back({projectile.center(), projectile.end()});  // 연출: 그 자리에서 반짝
        }
    }
    std::erase_if(projectiles_, [](const Projectile& p) { return !p.alive(); });
}

void FieldCombat::record_hits(const std::vector<HitEvent>& hits) {
    for (const auto& hit : hits) {
        hits_.push_back(hit);
        last_hit_ = hit;
        log::debug("Hit target {} for {}{}{}", hit.target, hit.amount, hit.critical ? " (critical)" : "",
                   hit.knocked_down ? " (knocked down)" : "");
    }
}

// 덮치는 몬스터마다: 판정이 주인공 맞는 상자에 닿고, 주인공이 무적이 아니고, 이번 덮치기에서 처음이면 맞아요.
void FieldCombat::resolve_monster_attacks(PlayerMotion& player) {
    for (auto& monster : monsters_) {
        const auto hitbox = monster.attack_hitbox();             // 덮치는 동안만 있어요
        const combat::AttackData* attack = monster.attack();
        if (!hitbox || attack == nullptr) continue;
        const engine::RectF hurtbox = player.hurtbox();
        if (!hitbox->intersects(hurtbox)) continue;              // 안 닿았어요
        // 무적(맞은 직후·구르기 처음)이면 이번 박자는 안 맞아요. 덮치기는 "아직 안 맞힘"으로 남아서
        // 무적이 끝났는데 아직 덮치는 중이면 그때 맞을 수 있어요.
        if (!player.can_be_hit()) continue;
        if (!monster.claim_hit()) continue;                      // 이번 덮치기에서 이미 맞혔어요
        combat::DamageInput input;                               // 피해 계산 (명세 9.3)
        input.attack_power = monster.data().attack_power;        // 슬라임 공격력 4
        input.power = attack->power;                             // 공격 위력 1.0
        input.defense = kHeroDefense;
        const combat::DamageResult damage = combat::compute_damage(input);
        const engine::Vec2 direction = monster.attack_direction();  // 덮친 쪽으로 밀려나요
        HeroHitEvent hit;
        hit.amount = player.take_hit(damage.amount, direction, attack->knockback);  // HP·경직·무적·넉백
        hit.fatal = player.dead();                               // 이 한 방에 쓰러졌나 (12b)
        hit.at = {hurtbox.x + hurtbox.w * 0.5f, hurtbox.y + hurtbox.h * 0.5f};
        hit.direction = direction;
        hit.knockback = attack->knockback;
        hit.hitstop = attack->hitstop;
        hit.shake = attack->shake;
        hero_hits_.push_back(hit);
        log::debug("Hero hit by {} for {} (hp {}/{})", monster.name(), hit.amount, player.health().current(),
                   player.health().max());
    }
}

// 겹친 몸을 부드럽게 떼어 놓아요. 한 쌍마다 한 박자에 kSeparationSpeed × dt까지만 (밀어내기 힘 제한).
// 순서가 늘 같아서(주인공-몬스터 순서대로, 그다음 몬스터끼리) 같은 입력이면 같은 결과예요.
void FieldCombat::separate(PlayerMotion& player, const world::MapData& map, float dt_seconds) {
    const float max_step = kSeparationSpeed * dt_seconds;  // 히트스톱(0)이면 밀지 않아요
    if (!(max_step > 0.0f)) return;
    const float hero_radius = player.mover().tuning().body.width * 0.5f;  // 몸 반지름 = 발 상자 가로의 절반
    for (auto& monster : monsters_) {                                     // 주인공 ↔ 몬스터
        if (monster.defeated() || player.dead()) continue;                // 쓰러진 몸은 밀지 않아요 (11d, 12b)
        const SeparationPush push = separate_bodies(
            {player.mover().feet(), hero_radius, kHeroPushWeight},
            {monster.feet(), monster.body().width * 0.5f, kMonsterPushWeight}, max_step);
        player.shove(push.first, map);   // 벽에 닿으면 벽을 따라 미끄러져요
        monster.shove(push.second, map);
    }
    for (std::size_t i = 0; i < monsters_.size(); ++i) {                  // 몬스터 ↔ 몬스터
        for (std::size_t j = i + 1; j < monsters_.size(); ++j) {
            auto& a = monsters_[i];
            auto& b = monsters_[j];
            if (a.defeated() || b.defeated()) continue;                   // 쓰러진 몸은 밀지 않아요 (11d)
            const SeparationPush push = separate_bodies({a.feet(), a.body().width * 0.5f, kMonsterPushWeight},
                                                        {b.feet(), b.body().width * 0.5f, kMonsterPushWeight},
                                                        max_step);
            a.shove(push.first, map);
            b.shove(push.second, map);
        }
    }
}

engine::Vec2 FieldCombat::spell_hand(engine::Vec2 feet, world::Direction facing) {
    const engine::Vec2 ground = feet + direction_vector(facing) * kSpellSpawnForward;  // 발 조금 앞
    return {ground.x, ground.y - kSpellLift};                                         // 가슴 높이
}

float FieldCombat::strongest_hitstop() const {
    float longest = 0.0f;  // 아무도 안 맞았으면 0 (멈추지 않아요)
    for (const auto& hit : hits_) longest = std::max(longest, hit.hitstop);
    for (const auto& hit : hero_hits_) longest = std::max(longest, hit.hitstop);
    return longest;
}

bool FieldCombat::hero_fell() const {
    return std::ranges::any_of(hero_hits_, [](const HeroHitEvent& hit) { return hit.fatal; });
}

std::string FieldCombat::debug_text() const {
    std::string text = "dummies " + std::to_string(dummies_.size());          // 허수아비 수
    if (!monsters_.empty()) {  // 몬스터 수와 "가장 바쁜" 상태 (11b: 싸우는 몬스터가 있으면 그 상태가 보여요)
        // 이름표 순서: 쉬기 < 배회 < 알아채기 < 쫓기 < 예고 < 공격 < 후딜 < 움찔 < 복귀 < 쓰러짐.
        // 싸움 쪽(알아채기~움찔)을 먼저 보여 줘요.
        const auto busy = [](MonsterState s) {
            const bool fighting = s >= MonsterState::Notice && s <= MonsterState::Hurt;
            return (fighting ? 100 : 0) + static_cast<int>(s);  // 싸우는 중이면 크게
        };
        const auto top = std::ranges::max_element(
            monsters_, [&busy](const Monster& a, const Monster& b) { return busy(a.state()) < busy(b.state()); });
        text += "  mobs " + std::to_string(monsters_.size()) + " (" + std::string(to_string(top->state())) + ")";
    }
    text += "  shards " + std::to_string(projectiles_.size());                // 날고 있는 빛 탄 수
    if (!last_hit_) return text;                                              // 아직 아무도 안 맞았으면 여기까지
    text += "  last hit " + std::to_string(last_hit_->amount) + (last_hit_->critical ? "!" : "") +  // ! = 치명타
            " -> #" + std::to_string(last_hit_->target);
    const auto index = static_cast<std::size_t>(last_hit_->target);           // 대상 번호는 1부터
    if (index >= 1 && index <= dummies_.size()) {                             // 그 허수아비의 지금 HP
        const auto& dummy = dummies_[index - 1];
        text += " " + std::to_string(dummy.hp()) + "/" + std::to_string(dummy.max_hp());
        if (dummy.down()) text += " (down)";                                  // 쓰러져 있으면 표시
    } else if (index > dummies_.size() && index <= dummies_.size() + monsters_.size()) {  // 몬스터 (11d)
        const auto& monster = monsters_[index - dummies_.size() - 1];
        text += " " + std::to_string(monster.hp()) + "/" + std::to_string(monster.max_hp());
        if (monster.defeated()) text += " (dead)";
    }
    return text;
}

}  // namespace wind::game
