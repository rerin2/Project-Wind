// 이 파일은 무엇을 하나요?
// 필드 화면의 효과 파일들을 읽고, 실패한 것은 기록만 하고 넘어가는 곳이에요.

#include "game/presentation/field_effects.hpp"

#include "engine/fx/particle_effect_loader.hpp"

#include "wind/content/error_list.hpp"
#include "wind/log.hpp"

#include <string_view>
#include <utility>

namespace wind::game {
namespace {

// 먼지 효과 파일 (콘텐츠 폴더 기준)
constexpr std::string_view kDustWalkFx = "fx/dust_walk.json";
constexpr std::string_view kDustDodgeFx = "fx/dust_dodge.json";
// 시험용 부하 효과의 수명(초). 부하 개수 ÷ 수명 = 1초에 만들 개수
constexpr float kStressLifetime = 4.0f;

// 효과 파일 하나를 읽어요. 잘못된 파일이면 이유를 기록하고 "없음"을 돌려줘요.
std::optional<engine::ParticleEffect> load_effect(const std::filesystem::path& content_root,
                                                  std::string_view path) {
    auto result = engine::load_particle_effect_file(content_root, path);
    if (!result.ok()) {
        content::log_content_errors("particle effect " + std::string(path), result.errors);
        return std::nullopt;
    }
    return std::move(*result.value);
}

// 성능 시험용 부하 효과: 화면 곳곳에 작은 점이 약 count개 떠 있게 만들어요. (--fx-stress)
engine::ParticleEffect make_stress_effect(std::uint32_t count) {
    engine::ParticleEffect effect;
    effect.id = "stress";
    effect.shape = engine::ParticleShape::Soft;
    effect.lifetime = {kStressLifetime, kStressLifetime};
    effect.speed = {4.0f, 12.0f};
    effect.spread = 360.0f;
    effect.start_size = {3.0f, 5.0f};
    effect.end_size = 2.0f;
    effect.start_color = {255, 240, 168, 200};
    effect.end_color = {255, 240, 168, 0};
    effect.rate = static_cast<float>(count) / kStressLifetime;
    return effect;
}

}  // namespace

FieldEffects load_field_effects(const std::filesystem::path& content_root,
                                const world::Ambience& ambience, const world::WeatherTable* weather,
                                std::uint32_t stress_particles) {
    FieldEffects effects;
    effects.dust_walk = load_effect(content_root, kDustWalkFx);
    effects.dust_dodge = load_effect(content_root, kDustDodgeFx);
    for (const auto& path : ambience.ambient_fx) {
        if (auto effect = load_effect(content_root, path)) {
            effects.ambient.push_back(std::move(*effect));
        }
    }
    for (const auto& path : ambience.night_fx) {
        if (auto effect = load_effect(content_root, path)) {
            effects.night.push_back(std::move(*effect));
        }
    }
    // 날씨 효과 (빗줄기, 물 튀김, 눈): 같은 파일은 한 번만 읽어요.
    if (weather != nullptr) {
        for (const auto& state : weather->states) {
            for (const auto* list : {&state.fx, &state.ground_fx}) {
                for (const auto& path : *list) {
                    if (!effects.weather_fx.contains(path)) {
                        if (auto effect = load_effect(content_root, path)) {
                            effects.weather_fx.emplace(path, std::move(*effect));
                        }
                    }
                }
            }
        }
    }
    // 빛 자리 효과(횃불 불티 등): 같은 파일은 한 번만 읽어요.
    for (const auto& light : ambience.lights) {
        if (!light.fx.empty() && !effects.light_fx.contains(light.fx)) {
            if (auto effect = load_effect(content_root, light.fx)) {
                effects.light_fx.emplace(light.fx, std::move(*effect));
            }
        }
    }
    if (stress_particles > 0) {
        log::info("Particle stress test: about {} particles", stress_particles);
        effects.ambient.push_back(make_stress_effect(stress_particles));
    }
    return effects;
}

}  // namespace wind::game
