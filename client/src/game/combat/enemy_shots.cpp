// 이 파일은 무엇을 하나요?
// 몬스터 침: 쏘기, 날리기, 주인공에게 닿으면 맞히기.

#include "game/combat/enemy_shots.hpp"

#include "game/combat/hero_strike.hpp"
#include "game/entities/player_motion.hpp"
#include "game/monsters/monster.hpp"

#include "wind/log.hpp"

#include <algorithm>
#include <utility>

namespace wind::game {
namespace {

// 침이 나오는 곳: 발에서 앞으로 8픽셀, 화면 위로 8픽셀 (개구리 입 높이)
constexpr float kMouthForward = 8.0f;
constexpr float kMouthLift = 8.0f;

}  // namespace

void EnemyShots::fire(const Monster& monster) {
    const combat::AttackData* attack = monster.attack();
    if (attack == nullptr || !attack->projectile) return;  // 날아가는 공격이 아니에요
    ProjectileShot shot;
    shot.direction = monster.attack_direction();                          // 예고 때 정한 방향
    shot.ground = monster.feet() + shot.direction * kMouthForward;
    shot.lift = kMouthLift;
    shot.data = *attack->projectile;                                      // 빠르기·거리·크기
    shot.attack_power = monster.data().attack_power;
    shot.power = attack->power;
    shot.knockback = attack->knockback;
    shot.hitstop = attack->hitstop;
    shot.shake = attack->shake;
    live_.push_back({Projectile(shot), monster.data().attack_power, *attack});
}

void EnemyShots::step(float dt_seconds, const world::MapData& map, PlayerMotion& player) {
    hero_hits_.clear();
    ended_.clear();
    for (auto& shot : live_) {
        // 침 네모가 주인공 맞는 상자에 닿았나: 무적이면 그냥 지나가요 (구르기로 피하기)
        const Projectile::HitTest hit_test = [&](const engine::RectF& rect) {
            if (!player.can_be_hit() || !rect.intersects(player.hurtbox())) return false;
            hero_hits_.push_back(strike_hero(player, shot.attack_power, shot.attack, shot.projectile.shot().direction));
            log::debug("Hero hit by a shot for {} (hp {}/{})", hero_hits_.back().amount, player.health().current(),
                       player.health().max());
            return true;  // 맞혔으니 침은 사라져요
        };
        shot.projectile.step(dt_seconds, map, hit_test);
        if (!shot.projectile.alive()) ended_.push_back({shot.projectile.center(), shot.projectile.end()});
    }
    std::erase_if(live_, [](const Shot& s) { return !s.projectile.alive(); });
    shots_.clear();  // 그리기용 목록을 새로
    for (const auto& shot : live_) shots_.push_back(shot.projectile);
}

}  // namespace wind::game
