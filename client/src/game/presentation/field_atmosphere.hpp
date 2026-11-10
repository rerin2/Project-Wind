#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 "하늘과 공기" 담당이에요. (명세 6.4, 6.5)
// - 하루 색 곡선을 보고 지금 시각의 세상 색과 밤 정도를 구해요 (바깥 맵만).
// - 날씨를 돌리고, 날씨 세기만큼 빗줄기·눈·물 튀김을 뿜고, 세상 색을 물들이고, 안개를 그려요.
// - 밤 정도만큼 반딧불 같은 밤 효과를 뿜어요.
// 동굴처럼 하루 색 곡선이 없는 맵은 시간과 날씨의 영향을 받지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_emitter.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/render/render_queue.hpp"
#include "game/presentation/ambient_particles.hpp"
#include "game/presentation/fog_layer.hpp"
#include "game/presentation/weather_system.hpp"

#include "wind/world/day_cycle.hpp"
#include "wind/world/weather_table.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::game {

// 이번 화면의 날씨·시간 상태 (조명과 디버그 화면이 써요)
struct AtmosphereConditions {
    std::optional<world::Rgba> ambient;  // 바깥 맵: 시간 색 × 날씨 색. 실내 맵: 없음(맵 고정 색을 써요)
    float night = 0.0f;                  // 밤 정도 0~1 (실내는 0)
};

class FieldAtmosphere {
public:
    // weather_fx / night_fx: 효과들. 이 물건보다 오래 살아 있어야 해요.
    FieldAtmosphere(std::optional<world::DayCycle> day_cycle,
                    std::optional<world::WeatherTable> weather,
                    const std::map<std::string, engine::ParticleEffect>& weather_fx,
                    const std::vector<engine::ParticleEffect>& night_fx, std::uint32_t seed,
                    std::string_view start_weather);
    FieldAtmosphere(const FieldAtmosphere&) = delete;
    FieldAtmosphere& operator=(const FieldAtmosphere&) = delete;

    // hour: 지금 게임 시각 (0~24), view: 카메라가 보는 맵 영역
    void update(float dt_seconds, float hour, const engine::RectF& view,
                engine::ParticleRandom& random, engine::ParticlePool& air_fx,
                engine::ParticlePool& ground_fx);
    // 다음 날씨로 빠르게 바꿔요 (디버그 F6). 무엇에서 무엇으로 바뀌는지 기록해요.
    void next_weather();
    // 안개를 그리기 목록에 넣어요. texture: 부드러운 둥근 그림
    void queue(const engine::Camera2D& camera, engine::TextureHandle texture,
               engine::RenderQueue& queue) const;

    [[nodiscard]] const AtmosphereConditions& conditions() const { return conditions_; }
    [[nodiscard]] bool outdoors() const { return day_cycle_.has_value(); }
    // 디버그 화면용: 날씨 이름("clear" 등, 날씨 표가 없으면 "none")과 세기.
    // 바뀌는 중이면 "clear -> rain"처럼 다음 날씨도 보여 줘요.
    [[nodiscard]] std::string weather_name() const;
    [[nodiscard]] float weather_intensity() const { return weather_.intensity(); }

private:
    // 날씨 하나가 쓰는 방출기들
    struct WeatherEmitters {
        std::vector<engine::ParticleEmitter> sky;     // 공중 (빗줄기, 눈)
        std::vector<engine::ParticleEmitter> ground;  // 땅 (물 튀김)
    };

    std::optional<world::DayCycle> day_cycle_;
    std::optional<world::WeatherTable> table_;  // weather_보다 먼저 있어야 해요 (weather_가 빌려 봐요)
    WeatherSystem weather_;
    std::vector<WeatherEmitters> emitters_;     // 날씨 표의 날씨 순서대로
    AmbientParticles night_;
    FogLayer fog_;
    AtmosphereConditions conditions_;
};

}  // namespace wind::game
