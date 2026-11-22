#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터가 쏜 침(날아가는 공격)들이에요. (명세 9.2, 13c 개구리)
// - 몬스터 공격 데이터에 "projectile"이 있으면, 공격이 시작되는 박자에 한 방을 쏴요 (Monster::take_shot)
// - 침은 주인공 빛 탄과 같은 Projectile로 날아가요 (벽·거리 끝에서 사라짐, 잘게 나눠 검사)
// - 주인공 맞는 상자에 닿으면, 무적이 아니면 맞히고 사라져요 (strike_hero, 덮치기와 같은 피해 규칙)
// - 무적(구르기·피격 직후·쉬는 중)이면 그냥 지나가요. 그래서 구르기로 피할 수 있어요.
// 게임 규칙이라 SDL을 몰라요.

#include "game/combat/hero_hit_event.hpp"
#include "game/combat/projectile.hpp"

#include "wind/combat/attack.hpp"
#include "wind/world/map.hpp"

#include <vector>

namespace wind::game {

class Monster;
class PlayerMotion;

class EnemyShots {
public:
    // 몬스터의 지금 공격으로 침 한 방을 쏴요 (공격 방향으로, 입 높이에서)
    void fire(const Monster& monster);
    // 한 박자: 날리고, 주인공에게 맞은 일과 사라진 침을 적어요 (지난 박자 기록은 지워요)
    void step(float dt_seconds, const world::MapData& map, PlayerMotion& player);

    [[nodiscard]] const std::vector<Projectile>& shots() const { return shots_; }
    // 이번 박자에 주인공이 침에 맞은 일
    [[nodiscard]] const std::vector<HeroHitEvent>& hero_hits() const { return hero_hits_; }
    // 이번 박자에 사라진 침 (연출: 그 자리에서 톡 터짐)
    [[nodiscard]] const std::vector<ProjectileEndEvent>& ended() const { return ended_; }

private:
    struct Shot {
        Projectile projectile;
        float attack_power = 0.0f;       // 쏜 몬스터의 공격력
        combat::AttackData attack;       // 위력·넉백·멈칫·흔들림
    };
    std::vector<Shot> live_;                   // 날고 있는 침과 그 피해 정보
    std::vector<Projectile> shots_;            // 그리기용 (live_와 같은 순서)
    std::vector<HeroHitEvent> hero_hits_;
    std::vector<ProjectileEndEvent> ended_;
};

}  // namespace wind::game
