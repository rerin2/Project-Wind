// 이 파일은 무엇을 하나요?
// 필드 룬 담당: 룬 받기, 쓰러지면 잃고 흔적 남기기, 흔적 되찾기, 흔적 연출.

#include "game/states/field_runes.hpp"

#include "game/entities/player_motion.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/states/field_combat.hpp"

namespace wind::game {
namespace {

constexpr float kBodyLift = 10.0f;  // 발에서 몸 가운데까지 (알갱이가 빨려 드는 곳)

}  // namespace

FieldRunes::FieldRunes(HeroProgress& progress, std::string_view map, engine::TextureHandle glow,
                       const engine::ParticleEffect* motes)
    : progress_(progress), map_(map), view_(glow, motes) {}

std::optional<engine::Vec2> FieldRunes::trace_here() const {
    const auto& trace = progress_.trace();
    if (!trace || trace->map != map_) return std::nullopt;  // 다른 맵의 흔적은 여기서 안 보여요
    return trace->at;
}

void FieldRunes::step(const FieldCombat& combat, const PlayerMotion& hero) {
    progress_.earn(combat.runes_earned());  // 1) 쓰러뜨린 몬스터의 룬 (13a)
    const engine::Vec2 feet = hero.mover().feet();
    if (combat.hero_fell()) {               // 2) 쓰러졌어요: 절반을 잃고 쓰러진 자리에 흔적 (14e)
        (void)progress_.fall(map_, feet);
        return;
    }
    // 3) 되찾기: 살아 있고 쉬는 중이 아니고, 흔적 가까이 서면 (한 번만, HeroProgress가 지워요)
    const auto at = trace_here();
    if (!at || hero.dead() || hero.rest().active()) return;
    if (engine::length(*at - feet) > kRecoverRadius) return;
    const std::int64_t got = progress_.recover_trace();
    recovered_ = got;
    recovered_fx_ = std::pair{*at, got};
}

void FieldRunes::update(float dt_seconds, engine::Vec2 feet, engine::ParticleRandom& random,
                        engine::ParticlePool& air_fx) {
    if (recovered_fx_) {  // 되찾은 박자 다음 화면: 흔적 자리에서 알갱이가 튀어나와요
        view_.recovered(recovered_fx_->first, recovered_fx_->second, random);
        recovered_fx_.reset();
    }
    view_.update(dt_seconds, trace_here(), feet - engine::Vec2{0.0f, kBodyLift}, random, air_fx);
}

}  // namespace wind::game
