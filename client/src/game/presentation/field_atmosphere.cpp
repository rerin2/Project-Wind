// 이 파일은 무엇을 하나요?
// 시간·날씨·안개·밤 효과를 매 화면 계산하는 곳이에요.

#include "game/presentation/field_atmosphere.hpp"

#include "wind/log.hpp"

#include <cmath>

namespace wind::game {
namespace {

// 빗줄기·눈이 생기는 영역: 화면보다 조금 넓게 (가장자리에서 갑자기 나타나지 않게)
constexpr float kWeatherMargin = 48.0f;

std::uint8_t multiply(std::uint8_t a, std::uint8_t b) {
    return static_cast<std::uint8_t>((static_cast<unsigned>(a) * b + 127u) / 255u);
}

}  // namespace

FieldAtmosphere::FieldAtmosphere(std::optional<world::DayCycle> day_cycle,
                                 std::optional<world::WeatherTable> weather,
                                 const std::map<std::string, engine::ParticleEffect>& weather_fx,
                                 const std::vector<engine::ParticleEffect>& night_fx,
                                 std::uint32_t seed, std::string_view start_weather)
    : day_cycle_(std::move(day_cycle)),
      table_(std::move(weather)),
      weather_(table_ ? &*table_ : nullptr, seed, start_weather) {
    // 날씨마다 쓸 방출기를 미리 만들어 둬요. (파일을 못 읽은 효과는 건너뛰어요)
    if (table_) {
        for (const auto& state : table_->states) {
            WeatherEmitters set;
            for (const auto& path : state.fx) {
                if (const auto it = weather_fx.find(path); it != weather_fx.end()) {
                    set.sky.emplace_back(it->second);
                }
            }
            for (const auto& path : state.ground_fx) {
                if (const auto it = weather_fx.find(path); it != weather_fx.end()) {
                    set.ground.emplace_back(it->second);
                }
            }
            emitters_.push_back(std::move(set));
        }
    }
    for (const auto& effect : night_fx) {
        night_.add(effect);
    }
}

void FieldAtmosphere::update(float dt_seconds, float hour, const engine::RectF& view,
                             engine::ParticleRandom& random, engine::ParticlePool& air_fx,
                             engine::ParticlePool& ground_fx) {
    weather_.update(dt_seconds);
    fog_.update(dt_seconds);

    // 1) 시간 색과 밤 정도 (바깥 맵만). 날씨 색을 곱해서 비 오는 날은 조금 어둡고 푸르게.
    conditions_ = {};
    if (day_cycle_) {
        const world::DaySample sample = world::sample_day_cycle(*day_cycle_, hour);
        const world::Rgba tint = weather_.tint();
        conditions_.ambient = world::Rgba{multiply(sample.ambient.r, tint.r),
                                          multiply(sample.ambient.g, tint.g),
                                          multiply(sample.ambient.b, tint.b), 255};
        conditions_.night = sample.night;
    }

    // 2) 날씨 효과: 지금 날씨의 방출기만, 세기만큼 뿜어요.
    const engine::RectF area{view.x - kWeatherMargin, view.y - kWeatherMargin,
                             view.w + kWeatherMargin * 2.0f, view.h + kWeatherMargin * 2.0f};
    if (weather_.current() != nullptr && weather_.current_index() < emitters_.size()) {
        auto& set = emitters_[weather_.current_index()];
        for (auto& emitter : set.sky) {
            emitter.update(dt_seconds, area, random, air_fx, weather_.intensity());
        }
        for (auto& emitter : set.ground) {
            emitter.update(dt_seconds, view, random, ground_fx, weather_.intensity());
        }
    }

    // 3) 밤 효과(반딧불): 밤 정도만큼
    if (conditions_.night > 0.0f) {
        night_.update(dt_seconds, view, random, air_fx, conditions_.night);
    }
}

void FieldAtmosphere::queue(const engine::Camera2D& camera, engine::TextureHandle texture,
                            engine::RenderQueue& queue) const {
    fog_.queue(camera, weather_.fog(), texture, queue);
}

std::string FieldAtmosphere::weather_name() const {
    const auto* state = weather_.current();
    if (state == nullptr) {
        return "none";
    }
    if (const auto* next = weather_.upcoming()) {
        return state->id + " -> " + next->id;
    }
    return state->id;
}

void FieldAtmosphere::next_weather() {
    if (weather_.current() == nullptr) {
        log::info("This map has no weather table; F6 does nothing here");
        return;
    }
    weather_.next();
    log::info("Weather: {}", weather_name());
}

}  // namespace wind::game
