// 이 파일은 무엇을 하나요?
// 필드의 칼 판정, 빛 탄 만들기·날리기, 허수아비 시간을 박자마다 진행하는 곳이에요.

#include "game/states/field_combat.hpp"

#include "game/entities/facing.hpp"

#include "wind/log.hpp"

#include <algorithm>
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

}  // namespace

FieldCombat::FieldCombat(std::vector<TrainingDummy> dummies) : dummies_(std::move(dummies)) {}

void FieldCombat::step(double dt_seconds, PlayerMotion& player, const world::MapData& map) {
    hits_.clear();   // 지난 박자 기록 지우기
    ended_.clear();
    const auto dt = static_cast<float>(dt_seconds);
    resolve_sword(player);         // 1) 칼
    spawn_spell(player);           // 2) 시전이 끝났으면 빛 탄 하나
    step_projectiles(dt, map);     // 3) 빛 탄 날리기
    for (auto& dummy : dummies_) { // 4) 쓰러진 허수아비가 다시 서기까지 시간 세기
        dummy.step(dt);
    }
}

// 칼이 판정 중이면 허수아비에 맞았는지 보고 피해를 줘요 (9c).
void FieldCombat::resolve_sword(PlayerMotion& player) {
    const auto hitbox = player.attack_hitbox();                 // 판정 중이 아니면 비어 있어요
    const auto* attack = player.attack().current();             // 지금 휘두르는 공격
    if (!hitbox || attack == nullptr || dummies_.empty()) {
        return;
    }
    const auto& weapon = player.attack().weapon();              // 든 무기
    Swing swing;                                                // 이번 칼 한 번
    swing.hitbox = *hitbox;                                     // 닿는 곳
    swing.attack_power = combat::physical_attack_power(weapon.base_power, weapon.strength_scaling,
                                                       kStrengthBonus);  // 공격력
    swing.power = attack->power;                                // 콤보마다 다른 위력
    swing.register_hit = [&player](TargetId id) { return player.register_hit(id); };  // 한 번만 맞게
    record_hits(resolve_swing(swing, dummies_));                // 맞았는지 보고 피해 주기
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
            swing.register_hit = [&used](TargetId) { return !std::exchange(used, true); };  // 첫 대상만
            const auto hits = resolve_swing(swing, dummies_);
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
        log::debug("Hit dummy {} for {}{}", hit.target, hit.amount,
                   hit.knocked_down ? " (knocked down)" : "");
    }
}

engine::Vec2 FieldCombat::spell_hand(engine::Vec2 feet, world::Direction facing) {
    const engine::Vec2 ground = feet + direction_vector(facing) * kSpellSpawnForward;  // 발 조금 앞
    return {ground.x, ground.y - kSpellLift};                                         // 가슴 높이
}

std::string FieldCombat::debug_text() const {
    std::string text = "dummies " + std::to_string(dummies_.size());          // 허수아비 수
    text += "  shards " + std::to_string(projectiles_.size());                // 날고 있는 빛 탄 수
    if (!last_hit_) return text;                                              // 아직 아무도 안 맞았으면 여기까지
    text += "  last hit " + std::to_string(last_hit_->amount) + " -> #" + std::to_string(last_hit_->target);
    const auto index = static_cast<std::size_t>(last_hit_->target);           // 대상 번호는 1부터
    if (index >= 1 && index <= dummies_.size()) {                             // 그 허수아비의 지금 HP
        const auto& dummy = dummies_[index - 1];
        text += " " + std::to_string(dummy.hp()) + "/" + std::to_string(dummy.max_hp());
        if (dummy.down()) text += " (down)";                                  // 쓰러져 있으면 표시
    }
    return text;
}

}  // namespace wind::game
