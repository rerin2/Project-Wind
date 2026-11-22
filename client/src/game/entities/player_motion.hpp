#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 몸 움직임 규칙을 한데 묶어요. (명세 7.2, 7.2.1, 8.5, 9.2)
// - Mover(다리): 걷기, 달리기, 구르기, 벽 미끄러짐
// - Stamina(기력): 달리기·구르기·공격에 쓰고, 쉬면 다시 참
// - Mana(MP): 마술에 쓰고, 늘 1초에 2씩 다시 참
// - AttackRunner(칼 휘두르기): 준비 -> 판정 -> 후딜, 콤보
// - SpellCaster(마술 시전): 시전 시간을 모았다가 마술을 내보냄
// - InputBuffer(버튼 메모장): 구르기·공격·마술 버튼을 0.15초 기억
// - Health(HP)와 HitReaction(경직·무적·넉백): 맞으면 HP가 줄고, 잠깐 굳고, 밀려나고, 잠깐 무적이에요 (11c)
// 한 박자 순서:
//   1) 구르기 메모가 있고, 구를 수 있으면 -> 구르기 (스태미나 25 씀)
//      공격 중이면 취소 구간에서만 굴러요 (공격은 취소돼요). 시전 중에는 못 굴러요 (명세 8.5)
//   2) 공격 메모가 있고, 휘두를 수 있고, 지침이 아니고 스태미나가 0보다 많으면
//      -> 공격 시작 또는 다음 콤보 (무기의 공격 스태미나만큼 씀, 모자라면 0이 돼요)
//   3) 마술 메모가 있고, 시전할 수 있으면:
//      MP가 충분하면 -> MP를 먼저 내고 시전 시작 (방향은 이때 정해져요)
//      MP가 모자라면 -> 메모를 지우고 "MP 모자람" 횟수를 하나 올려요 (HUD가 깜빡여요)
//   4) 공격 중에는 발이 멈추고, 시전 중에는 천천히 걸어요(0.4배). 둘 다 달리지 않아요
//   5) Shift를 누르고 움직이며 지침이 아니면 -> 달리기 (달리는 동안 1초에 25 씀)
//   6) 스태미나는 쓰지 않았으면 0.6초 뒤부터 다시 참, MP는 박자마다 조금씩 참
//   7) 시전이 끝난 박자에는 spell_released()가 true예요 (FieldCombat이 빛 탄을 만들어요)
//   8) 경직 중에는 상태가 hit이라 표가 모든 행동을 막고, 다리도 멈추고, 넉백만큼 밀려나요 (벽 미끄러짐, 11c)
//   9) HP가 0이면 쓰러짐(dead, 12b): 버튼·다리·스태미나·MP가 모두 멈추고 맞지도 않아요.
//      마지막 한 방의 넉백만 끝까지 밀려나요. 일어나기(부활)는 맵을 새로 여는 것으로 해요 (12c).
//  10) 쿠션에서 쉬는 중(resting, 13b)이면 버튼이 안 먹고 다리가 멈추고 맞지 않아요. 방향키를 누르면 기지개 켜고 일어나요.
// 어떤 상태에서 무엇을 시작할 수 있는지는 상태 전이 표(character_state)를 따라요.
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/combat/attack_runner.hpp"
#include "game/combat/spell_caster.hpp"
#include "game/entities/character_state.hpp"
#include "game/entities/health.hpp"
#include "game/entities/hero_vitals.hpp"
#include "game/entities/hero_rest.hpp"
#include "game/entities/hit_reaction.hpp"
#include "game/entities/mana.hpp"
#include "game/entities/mover.hpp"
#include "game/entities/stamina.hpp"
#include "game/input/input_buffer.hpp"

#include "wind/combat/weapon.hpp"
#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

#include <optional>

namespace wind::game {

// "한 번도 맞지 않음"을 나타내는 아주 긴 시간 (초). 쉬기 조건(3초)을 늘 통과해요.
inline constexpr float kNeverHitSeconds = 1.0e9f;

class PlayerMotion {
public:
    // buffer_seconds: 구르기·공격 버튼을 몇 초 동안 기억할지
    // weapon: 든 무기 (콤보가 비어 있으면 공격을 못 해요)
    // mana: MP 숫자 (9단계는 최대 50 고정), health: HP 숫자, hit: 경직·무적·맞는 상자 (11c)
    PlayerMotion(engine::Vec2 feet, world::Direction facing, MovementTuning movement,
                 StaminaTuning stamina, double buffer_seconds, combat::Weapon weapon = {},
                 ManaTuning mana = {}, HealthTuning health = {}, HitTuning hit = {});

    // 구르기 버튼을 눌렀어요 (화면 한 장마다, 박자 계산 전에). 메모장에 적어 둬요.
    void press_dodge();
    // 공격 버튼을 눌렀어요. 메모장에 적어 둬요.
    void press_attack();
    // 마술 버튼을 눌렀어요 (L / C). 메모장에 적어 둬요.
    void press_cast();
    // 고른 마술을 정해요 (없으면 마술을 못 써요)
    void set_spell(std::optional<combat::Spell> spell) { caster_.set_spell(std::move(spell)); }
    // 한 박자. move: 가고 싶은 방향, run_held: Shift를 누르고 있나
    // 구르기를 시작했으면 true
    bool step(double dt_seconds, engine::Vec2 move, bool run_held, const world::MapData& map);

    // 지금 판정 구간이면 공격이 닿는 네모 (맵 위 픽셀), 아니면 nullopt
    [[nodiscard]] std::optional<engine::RectF> attack_hitbox() const;
    // 이번 휘두르기에서 이 대상을 처음 맞혔으면 true (판정 구간에서만)
    bool register_hit(TargetId target) { return attack_.register_hit(target); }

    // 지금 맞을 수 있나요? 피격 무적(맞은 뒤 0.6초)이나 구르기 무적(구르기 처음 0.16초)이면 안 맞아요 (11c)
    // 쓰러져 있어도 안 맞아요 (12b)
    [[nodiscard]] bool can_be_hit() const;
    // 맞았어요 (FieldCombat이 불러요): HP를 깎고, 하던 공격·시전·구르기를 끊고, 경직·무적·넉백을 시작해요.
    // direction: 밀려날 쪽 (길이 1), knockback: 밀어내는 힘 (픽셀/초). 실제로 깎인 HP를 돌려줘요.
    // 쓰러져 있으면 아무것도 안 하고 0이에요.
    int take_hit(int amount, engine::Vec2 direction, float knockback);
    // HP가 0이라 쓰러져 있나요? (12b)
    [[nodiscard]] bool dead() const { return health_.empty(); }
    // 지금 HP·MP·스태미나 (12c: 맵을 옮길 때 새 필드로 들고 가요)
    [[nodiscard]] HeroVitals vitals() const;
    // 들고 온 HP·MP·스태미나로 맞춰요 (12c). 각 통이 범위 안으로 잘라요.
    void restore(const HeroVitals& vitals);
    // 캣닢 쿠션에서 쉬기 시작 (13b): HP·MP·스태미나를 가득 채우고, seat(쿠션 자리)로 옮겨 가 앉아요.
    // 서 있거나 걷는 중에만 돼요 (공격·구르기·경직·쓰러짐이면 false). 쉬는 동안은 맞지 않아요.
    bool begin_rest(engine::Vec2 seat);
    // 쿠션 위에서 앉은 채 시작해요 (쉬기·부활로 새 맵을 연 직후, 13b)
    void sit_on(engine::Vec2 seat);
    // 앉아 있으면 기지개 켜고 일어나요 (E, 13b). 방향키는 step이 알아서 일으켜요.
    bool stand_up() { return rest_.stand_up(); }
    [[nodiscard]] const HeroRest& rest() const { return rest_; }
    // 레벨업으로 최대 HP·MP·스태미나가 바뀌었어요 (13c): 새 최대로 바꾸고 모두 가득 채워요.
    void raise_maximums(int max_hp, float max_mp, float max_stamina);
    // 마지막으로 맞은 뒤 지난 시간 (쉬기 조건: 3초, 13b). 한 번도 안 맞았으면 아주 큰 값이에요.
    [[nodiscard]] float seconds_since_hit() const { return seconds_since_hit_; }
    // 맵 위의 맞는 상자 (픽셀)
    [[nodiscard]] engine::RectF hurtbox() const;
    // 밖에서 밀어요 (11c: 몸끼리 밀어내기). 벽에 닿으면 벽을 따라 미끄러져요.
    void shove(engine::Vec2 delta, const world::MapData& map) { mover_.shove(delta, map); }

    // 지금 상태 (idle, moving, running, attacking, casting, dodging, hit, dead)
    [[nodiscard]] CharacterState state() const;
    // 이번 박자에 공격(또는 다음 콤보)을 시작했나 (연출용)
    [[nodiscard]] bool attack_started() const { return attack_started_; }
    // 이번 박자에 시전이 끝나 마술이 나갔나 (FieldCombat이 빛 탄을 만들어요)
    [[nodiscard]] bool spell_released() const { return spell_released_; }
    // 마술을 쓸 방향 (시전을 시작할 때 정해요)
    [[nodiscard]] world::Direction cast_facing() const { return cast_facing_; }
    // MP가 모자라서 마술이 안 나간 횟수 (늘기만 해요). HUD가 바뀐 걸 보고 MP 바를 깜빡여요.
    [[nodiscard]] unsigned mana_denied_count() const { return mana_denied_count_; }

    [[nodiscard]] const Mover& mover() const { return mover_; }
    [[nodiscard]] const Stamina& stamina() const { return stamina_; }
    [[nodiscard]] const Mana& mana() const { return mana_; }
    [[nodiscard]] const AttackRunner& attack() const { return attack_; }
    [[nodiscard]] const SpellCaster& caster() const { return caster_; }
    [[nodiscard]] const Health& health() const { return health_; }
    [[nodiscard]] const HitReaction& hit_reaction() const { return hit_; }

private:
    bool try_dodge();
    void try_attack(engine::Vec2 move);
    void try_cast(engine::Vec2 move);
    // 쓰러져 있는 한 박자: 다리를 세우고 마지막 넉백만 이어가요 (12b)
    void step_dead(double dt_seconds, const world::MapData& map);
    // 쿠션에서 쉬는 한 박자: 방향키면 일어나고, 쿠션 가운데로 옮겨 가요 (13b)
    void step_resting(double dt_seconds, engine::Vec2 move, const world::MapData& map);

    Mover mover_;
    Stamina stamina_;
    Mana mana_;
    AttackRunner attack_;
    SpellCaster caster_;
    InputBuffer buffer_;
    Health health_;
    HitReaction hit_;
    HeroRest rest_;                        // 쿠션 쉬기 시간표 (13b)
    float seconds_since_hit_ = kNeverHitSeconds;  // 마지막으로 맞은 뒤 지난 시간 (13b)
    bool attack_started_ = false;
    bool spell_released_ = false;
    world::Direction cast_facing_ = world::Direction::Down;
    unsigned mana_denied_count_ = 0;
};

}  // namespace wind::game
