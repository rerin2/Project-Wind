// 이 파일은 무엇을 하나요?
// 싸움 연출 담당들을 차례로 갱신하고 그리는 곳이에요. 맞은 일은 번쩍임·흔들림·숫자·불꽃으로 바꿔요.
// 값의 이유는 docs/VISUALS.md "Hit feel" 절에 있어요.

#include "game/presentation/field_combat_fx.hpp"

#include "engine/fx/particle_spawn.hpp"
#include "game/entities/facing.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace wind::game {
namespace {

// 피해 숫자는 맞은 상자 가운데보다 32픽셀 위(머리 위 HP 바 바로 위)에서 시작해요.
// 10·24로 해 보니 허수아비 머리 위 HP 바와 숫자가 겹쳐서 읽기 힘들었어요 (10b 화면 확인).
constexpr float kNumberLift = 32.0f;
constexpr int kCritSparkBursts = 2;   // 치명타는 불꽃을 두 번 터뜨려요
// 몬스터 사라짐 파티클은 발에서 이만큼 위(슬라임 몸 가운데쯤)에서 터져요 (11d)
constexpr float kVanishLift = 6.0f;
// 룬 알갱이가 빨려 드는 곳: 주인공 발에서 이만큼 위 (몸 가운데, 13c)
constexpr float kHeroBodyLift = 18.0f;
// 몬스터 침 (13c): 초록 방울 12픽셀 + 밝은 심 6픽셀
constexpr float kSpitSize = 12.0f;
constexpr float kSpitCore = 6.0f;
constexpr engine::Color kSpitColor{120, 200, 70, 230};
constexpr engine::Color kSpitCoreColor{220, 255, 170, 255};
// 달빛 칼날 칼자국 (15c): 칼보다 푸른 달빛 색, 조금 더 오래 보여요. docs/VISUALS.md "Moon blade" 절
constexpr engine::Color kMoonSlashTint{170, 205, 255, 255};
constexpr float kMoonSlashSeconds = 0.24f;
// 혜성 광선이 켜지는 순간 흔들림 (트라우마로 0.5, 약 1.5픽셀 잠깐). docs/VISUALS.md "Comet beam" 절
constexpr float kBeamOpenShake = 0.25f;

// 시뮬레이션 광선(HeroBeam)을 그릴 모양으로: 손끝은 그리는 발 위치(부드럽게 이어진 곳)에서 다시 재요
BeamShape beam_shape(const HeroBeam& beam, engine::Vec2 drawn_feet) {
    BeamShape shape;
    shape.active = beam.active();
    shape.hand = FieldCombat::spell_hand(drawn_feet, beam.facing());
    shape.direction = direction_vector(beam.facing());
    shape.length = beam.length();
    shape.width = beam.width();
    shape.blocked = beam.blocked();
    shape.hit_rect = beam.rect();
    shape.light = beam.light();
    return shape;
}

}  // namespace

FieldCombatFx::FieldCombatFx(CombatFxSetup setup)
    : view_(setup.shadow, std::move(setup.dummies)),
      monster_view_(setup.shadow, setup.glow, std::move(setup.monsters)),
      slash_(setup.slash),
      moon_slash_(setup.slash, kMoonSlashTint),
      spell_(setup.star_trail, setup.star_burst, setup.glow),
      beam_(setup.star_trail, setup.star_burst, setup.glow),
      vignette_texture_(setup.vignette),
      hit_spark_(setup.hit_spark),
      monster_fx_(setup.monster_fx),
      glow_(setup.glow),
      spit_splash_(setup.spit_splash) {}

void FieldCombatFx::set_settings(const FeelSettings& settings) {
    shake_.set_strength(settings.shake);
    view_.set_flash_strength(settings.flash);
    vignette_.set_strength(settings.flash);  // 붉은 비네트도 번쩍임 설정을 따라요 (11c)
    monster_view_.set_flash_strength(settings.flash);  // 몬스터 번쩍임·쓰러짐 깜빡임 (11d)
}

void FieldCombatFx::collect(const FieldCombat& combat) {
    const auto& hits = combat.hits();
    pending_hits_.insert(pending_hits_.end(), hits.begin(), hits.end());
    const auto& ends = combat.projectile_ends();
    pending_ends_.insert(pending_ends_.end(), ends.begin(), ends.end());
    const auto& hero_hits = combat.hero_hits();
    pending_hero_hits_.insert(pending_hero_hits_.end(), hero_hits.begin(), hero_hits.end());
    const auto& vanished = combat.vanished();
    pending_vanished_.insert(pending_vanished_.end(), vanished.begin(), vanished.end());
    const auto& drops = combat.rune_drops();  // 룬이 나온 자리 (13c)
    pending_drops_.insert(pending_drops_.end(), drops.begin(), drops.end());
    const auto& spit_ends = combat.enemy_shots().ended();  // 사라진 침 (13c)
    pending_spit_ends_.insert(pending_spit_ends_.end(), spit_ends.begin(), spit_ends.end());
    for (const auto& slash : combat.spell_slashes()) {  // 달빛 칼날: 벤 박자에 바로 띄워요 (15c)
        moon_slash_.play(slash.box, slash.facing, kMoonSlashSeconds);
        moon_light_ = slash.light;
        moon_at_ = {slash.box.x + slash.box.w * 0.5f, slash.box.y + slash.box.h * 0.5f};
    }
}

void FieldCombatFx::update(float dt_seconds, float alpha, const FieldCombat& combat,
                           const PlayerMotion& player, engine::Vec2 drawn_feet,
                           engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    for (const auto& hit : pending_hits_) {  // 1) 모아 둔 맞은 일을 연출로
        play_hit(hit, combat, random, air_fx);
    }
    pending_hits_.clear();
    for (const auto& hit : pending_hero_hits_) {  // 주인공이 맞았으면: 흔들림 + 붉은 비네트 (11c)
        shake_.add(hit.shake);
        vignette_.trigger();
    }
    pending_hero_hits_.clear();
    for (const auto& vanish : pending_vanished_) {  // 다 사라진 몬스터: 그 자리에서 파티클 (11d)
        play_vanish(vanish, combat, random, air_fx);
    }
    pending_vanished_.clear();
    for (const auto& drop : pending_drops_) runes_.spawn(drop.at, drop.amount, random);  // 룬 알갱이 톡 (13c)
    pending_drops_.clear();
    for (const auto& end : pending_spit_ends_) {  // 침이 사라진 자리에서 초록 방울이 톡 (13c)
        if (spit_splash_ != nullptr) (void)engine::emit_burst(*spit_splash_, end.at, 0.0f, random, air_fx);
    }
    pending_spit_ends_.clear();
    runes_.update(dt_seconds, {drawn_feet.x, drawn_feet.y - kHeroBodyLift});  // 주인공 몸 가운데로 빨려 들어요
    // 2) 마술: 시전 중이면 손끝 빛, 빛 탄 꼬리, 사라진 자리 반짝
    std::optional<CastGlow> cast;
    if (player.caster().busy() && !player.caster().beaming()) {  // 모으는 동안만 (광선은 BeamView가 그려요)
        const auto* spell = player.caster().spell();  // 시전 중인 마술의 빛 (15b)
        cast = CastGlow{FieldCombat::spell_hand(drawn_feet, player.cast_facing()), player.caster().progress(),
                        spell != nullptr ? spell->light : std::nullopt};
    }
    spell_.update(dt_seconds, combat.projectiles(), alpha, pending_ends_, cast, random, air_fx);
    pending_ends_.clear();
    if (beam_.update(dt_seconds, beam_shape(combat.beam(), drawn_feet), random, air_fx)) {
        shake_.add(kBeamOpenShake);  // 혜성 광선이 켜지는 순간 쿵 (15d)
    }
    // 3) 나머지 연출의 시간 흘리기: HP 바·번쩍임, 칼자국, 흔들림, 숫자
    view_.update(dt_seconds, combat.dummies());
    monster_view_.update(dt_seconds, combat.monsters());  // 숨쉬기·뜀 세기 (11a)
    slash_.update(dt_seconds, player.attack(), player.attack_hitbox(), player.mover().facing());
    moon_slash_.advance(dt_seconds);  // 달빛 칼날 칼자국 (15c)
    shake_.update(dt_seconds);
    vignette_.update(dt_seconds);
    numbers_.update(dt_seconds);
}

void FieldCombatFx::play_vanish(const MonsterVanishEvent& vanish, const FieldCombat& combat,
                                engine::ParticleRandom& random, engine::ParticlePool& air_fx) const {
    if (monster_fx_ == nullptr || vanish.index >= combat.monsters().size()) return;
    const auto& path = combat.monsters()[vanish.index].data().death_fx;  // 이 몬스터의 사라짐 효과 파일
    if (!path) return;                                                  // 효과가 없는 몬스터
    const auto it = monster_fx_->find(*path);
    if (it == monster_fx_->end()) return;                               // 파일을 못 읽었으면 효과만 빠져요
    // 몸 가운데 높이(발에서 조금 위)에서 사방으로 튀어요
    (void)engine::emit_burst(it->second, {vanish.at.x, vanish.at.y - kVanishLift}, 0.0f, random, air_fx);
}

void FieldCombatFx::play_hit(const HitEvent& hit, const FieldCombat& combat,
                             engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    view_.flash(combat.dummies(), hit.target);      // 하얀 번쩍임 (허수아비)
    monster_view_.flash(combat.monsters(), hit.target);  // 하얀 번쩍임 (몬스터, 11d)
    shake_.add(hit.shake);                          // 화면 흔들림 (트라우마)
    numbers_.spawn({hit.at.x, hit.at.y - kNumberLift}, hit.amount, hit.critical);  // 피해 숫자
    if (hit_spark_ == nullptr) {
        return;  // 불꽃 효과 파일이 없으면 불꽃만 빼요
    }
    // 불꽃은 밀린 방향으로 튀어요. 치명타는 두 번
    const float degrees = engine::direction_to_degrees(hit.direction);
    const int bursts = hit.critical ? kCritSparkBursts : 1;
    for (int i = 0; i < bursts; ++i) {
        (void)engine::emit_burst(*hit_spark_, hit.at, degrees, random, air_fx);
    }
}

void FieldCombatFx::queue(const engine::Camera2D& camera, const FieldCombat& combat,
                          const PlayerMotion& player, float alpha, bool show_boxes,
                          engine::RenderQueue& queue) const {
    // 허수아비(주인공과 발 위치로 줄 서기), HP 바, 번쩍임, F4면 판정 상자
    view_.queue(camera, combat.dummies(), player.attack_hitbox(), show_boxes, queue);
    monster_view_.queue(camera, combat.monsters(), alpha, show_boxes, queue);  // 몬스터 (11a)
    slash_.queue(camera, queue);                                        // 칼자국
    moon_slash_.queue(camera, queue);                                   // 달빛 칼날 (15c)
    spell_.queue(camera, combat.projectiles(), alpha, show_boxes, queue);  // 손끝 빛과 빛 탄
    beam_.queue(camera, show_boxes, queue);                            // 혜성 광선 (15d)
    runes_.queue(camera, glow_, queue);                                // 빨려 드는 룬 알갱이 (13c)
    queue_enemy_shots(camera, combat, alpha, queue);                   // 몬스터 침 (13c)
    vignette_.queue(vignette_texture_, camera.viewport(), queue);         // 피격 붉은 가장자리 (11c)
}

void FieldCombatFx::queue_enemy_shots(const engine::Camera2D& camera, const FieldCombat& combat, float alpha,
                                      engine::RenderQueue& queue) const {
    for (const auto& shot : combat.enemy_shots().shots()) {
        // 두 박자 사이를 이어 그려요 (부드럽게). 몸 높이(lift)만큼 위에
        const engine::Vec2 ground = engine::lerp(shot.previous_ground(), shot.ground(), std::clamp(alpha, 0.0f, 1.0f));
        const engine::Vec2 screen = camera.world_to_screen({ground.x, ground.y - shot.shot().lift});
        engine::SpriteDraw blob;  // 바깥 초록 방울
        blob.texture = glow_;
        blob.dest = {std::round(screen.x - kSpitSize * 0.5f), std::round(screen.y - kSpitSize * 0.5f), kSpitSize, kSpitSize};
        blob.tint = kSpitColor;
        queue.push_sprite(engine::DrawLayer::YSorted, ground.y, blob);
        engine::SpriteDraw core = blob;  // 안쪽 밝은 심 (빛나 보여서 밤에도 보여요)
        core.dest = {std::round(screen.x - kSpitCore * 0.5f), std::round(screen.y - kSpitCore * 0.5f), kSpitCore, kSpitCore};
        core.tint = kSpitCoreColor;
        core.blend = engine::BlendMode::Add;
        queue.push_sprite(engine::DrawLayer::Glow, ground.y, core);
    }
}

std::vector<engine::Light> FieldCombatFx::lights(const FieldCombat& combat, float alpha) const {
    auto result = spell_.lights(combat.projectiles(), alpha);         // 빛 탄, 손끝
    const auto warnings = monster_view_.lights(combat.monsters());    // 공격 예고 붉은 빛 (11b)
    if (moon_light_ && moon_slash_.visible()) {  // 달빛 칼날 빛: 벨 때 밝았다가 칼자국과 같이 사라져요 (15c)
        engine::Light light;
        light.position = moon_at_;
        light.radius = moon_light_->radius;
        light.color = {moon_light_->color.r, moon_light_->color.g, moon_light_->color.b, 255};
        light.intensity = moon_light_->intensity * moon_slash_.strength();
        result.push_back(light);
    }
    result.insert(result.end(), warnings.begin(), warnings.end());
    const auto beam = beam_.lights();  // 혜성 광선: 손끝·가운데·끝 (15d)
    result.insert(result.end(), beam.begin(), beam.end());
    return result;
}

}  // namespace wind::game
