// 이 파일은 무엇을 하나요?
// 맵 분위기 정보를 실제 빛으로 바꾸고, 매 화면 조명 준비물을 만드는 곳이에요.

#include "game/presentation/field_lighting.hpp"

#include "engine/light/light_drawing.hpp"

#include <random>

namespace wind::game {
namespace {

// 빛마다 깜빡임 출발점을 다르게: 번호 × 이 값 (나누어떨어지지 않는 수라 서로 잘 안 겹쳐요)
constexpr float kPhaseStep = 1.7f;
// 주인공 빛은 발이 아니라 가슴 높이에서 나와요 (픽셀)
constexpr float kPlayerLightLift = 14.0f;
// 불티가 생기는 작은 네모 크기 (픽셀)
constexpr float kFxSourceSize = 4.0f;
// 성능 시험용 빛을 흩어 놓을 범위 (중심에서 가로·세로 픽셀) — 화면 하나 크기
constexpr float kStressHalfWidth = 300.0f;
constexpr float kStressHalfHeight = 160.0f;
constexpr std::uint32_t kStressSeed = 5;

engine::Color to_color(world::Rgba c) {
    return {c.r, c.g, c.b, c.a};
}

}  // namespace

FieldLighting::FieldLighting(const world::Ambience& ambience, std::int32_t tile_size,
                             const std::map<std::string, engine::ParticleEffect>& light_fx,
                             std::uint32_t stress_lights, engine::Vec2 stress_center) {
    enabled_ = ambience.ambient_light.has_value();
    if (enabled_) {
        ambient_ = to_color(*ambience.ambient_light);
    }
    const auto size = static_cast<float>(tile_size);
    for (std::size_t i = 0; i < ambience.lights.size(); ++i) {
        const world::LightDef& def = ambience.lights[i];
        engine::Light light;
        light.position = {def.x * size, def.y * size};  // 칸 단위 -> 픽셀
        light.radius = def.radius;
        light.color = to_color(def.color);
        light.intensity = def.intensity;
        light.flicker = def.flicker;
        light.pulse_frequency = def.pulse_frequency;
        light.pulse_depth = def.pulse_depth;
        light.phase = static_cast<float>(i) * kPhaseStep;
        lights_.push_back(light);
        // 이 빛 자리에서 뿜을 효과가 있으면 방출기를 하나 붙여요.
        if (!def.fx.empty()) {
            if (const auto it = light_fx.find(def.fx); it != light_fx.end()) {
                fx_sources_.push_back({engine::ParticleEmitter(it->second), light.position});
            }
        }
    }
    if (ambience.player_light) {
        engine::Light light;
        light.radius = ambience.player_light->radius;
        light.color = to_color(ambience.player_light->color);
        light.intensity = ambience.player_light->intensity;
        player_light_ = light;
    }
    // 성능 시험용 빛: 화면 하나 크기 안에 따뜻한 색 빛을 흩어 놓아요.
    std::minstd_rand rng(kStressSeed);
    std::uniform_real_distribution<float> unit(-1.0f, 1.0f);
    std::uniform_real_distribution<float> radius(64.0f, 110.0f);
    for (std::uint32_t i = 0; i < stress_lights; ++i) {
        engine::Light light;
        light.position = {stress_center.x + unit(rng) * kStressHalfWidth,
                          stress_center.y + unit(rng) * kStressHalfHeight};
        light.radius = radius(rng);
        light.color = {255, 196, 128, 255};
        light.intensity = 0.8f;
        light.flicker = 0.2f;
        light.phase = static_cast<float>(lights_.size()) * kPhaseStep;
        lights_.push_back(light);
    }
}

void FieldLighting::update(float dt_seconds, engine::Vec2 player_feet, engine::ParticleRandom& random,
                           engine::ParticlePool& air_fx) {
    time_ += static_cast<double>(dt_seconds);
    if (player_light_) {
        player_light_->position = {player_feet.x, player_feet.y - kPlayerLightLift};
    }
    for (auto& source : fx_sources_) {
        const engine::RectF area{source.position.x - kFxSourceSize * 0.5f,
                                 source.position.y - kFxSourceSize * 0.5f, kFxSourceSize,
                                 kFxSourceSize};
        source.emitter.update(dt_seconds, area, random, air_fx);
    }
}

const engine::LightingFrame& FieldLighting::build(const engine::Camera2D& camera,
                                                  engine::TextureHandle texture) {
    frame_.enabled = enabled_ && texture.valid();
    frame_.ambient = ambient_;
    frame_.lights.clear();
    stats_ = {};
    if (!frame_.enabled) {
        return frame_;
    }
    // 주인공 빛을 맨 앞에 둬요. 빛이 너무 많아 잘릴 때도 주인공 빛은 꼭 그려지게요.
    visible_.clear();
    if (player_light_) {
        visible_.push_back(*player_light_);
    }
    visible_.insert(visible_.end(), lights_.begin(), lights_.end());
    stats_ = engine::queue_lights(visible_, camera, time_, texture, frame_);
    return frame_;
}

}  // namespace wind::game
