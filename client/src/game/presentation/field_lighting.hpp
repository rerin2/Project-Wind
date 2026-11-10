#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 조명 담당이에요. (명세 6.3)
// - 맵의 분위기 정보(주변광, 횃불 같은 빛)를 받아서 빛 목록을 만들어요.
// - 주인공을 따라다니는 작은 빛을 움직여요.
// - 횃불 자리에서 불티(파티클)를 계속 뿜어요.
// - 매 화면, 보이는 빛만 골라 렌더러에 줄 "조명 준비물"(LightingFrame)을 만들어요.
// 맵에 주변광이 없으면(밝은 낮 맵) 조명을 쓰지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_emitter.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/light/lighting_frame.hpp"

#include "wind/world/ambience.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace wind::game {

class FieldLighting {
public:
    // ambience: 맵 분위기, tile_size: 칸 크기(빛 위치는 칸 단위라 픽셀로 바꿔요)
    // light_fx: 빛 자리에서 뿜을 효과들 (파일 위치 -> 효과). 이 물건보다 오래 살아 있어야 해요.
    // stress_lights / stress_center: 성능 시험용 빛을 stress_center 주변에 몇 개 더 놓을지
    FieldLighting(const world::Ambience& ambience, std::int32_t tile_size,
                  const std::map<std::string, engine::ParticleEffect>& light_fx,
                  std::uint32_t stress_lights, engine::Vec2 stress_center);

    // 이번 화면의 날씨·시간 상태를 알려 줘요. (바깥 맵에서 매 화면)
    // ambient: 바깥 주변광 (있으면 맵의 고정 주변광 대신 써요), night: 밤 정도 0~1
    // night_only 빛(가로등)은 밤 정도만큼 밝아져요.
    void set_conditions(std::optional<world::Rgba> ambient, float night);

    // 시간이 흘렀어요: 깜빡임 시계를 돌리고, 주인공 빛을 옮기고, 불티를 뿜어요.
    void update(float dt_seconds, engine::Vec2 player_feet, engine::ParticleRandom& random,
                engine::ParticlePool& air_fx);
    // 이번 화면의 조명 준비물을 만들어요. texture: 둥근 빛 그림
    const engine::LightingFrame& build(const engine::Camera2D& camera, engine::TextureHandle texture);

    [[nodiscard]] bool enabled() const { return enabled_; }
    [[nodiscard]] const engine::LightStats& stats() const { return stats_; }
    [[nodiscard]] const std::vector<engine::Light>& lights() const { return lights_; }

private:
    // 빛 자리에서 계속 뿜는 효과 (횃불 불티 등)
    struct FxSource {
        engine::ParticleEmitter emitter;
        engine::Vec2 position;
    };

    bool fixed_enabled_ = false;                 // 맵에 고정 주변광이 있나 (동굴)
    bool enabled_ = false;                       // 이번 화면에 조명을 쓰나
    engine::Color fixed_ambient_{255, 255, 255, 255};
    engine::Color ambient_{255, 255, 255, 255};
    float night_ = 0.0f;
    std::vector<bool> night_only_;               // lights_와 같은 순서: 밤에만 켜지는 빛인가
    std::vector<engine::Light> lights_;           // 맵에 놓인 빛 (+ 시험용 빛)
    std::optional<engine::Light> player_light_;   // 주인공을 따라다니는 빛
    std::vector<FxSource> fx_sources_;
    std::vector<engine::Light> visible_;          // build 때 쓰는 임시 목록 (매번 새로 만들지 않으려고)
    double time_ = 0.0;                           // 깜빡임 시계 (초)
    engine::LightingFrame frame_;
    engine::LightStats stats_;
};

}  // namespace wind::game
