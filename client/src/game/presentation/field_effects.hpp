#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면이 쓰는 파티클 효과 요리법들을 한꺼번에 읽어 오는 도우미예요.
// - 먼지 두 가지 (걷기, 구르기)
// - 맵 분위기 효과 (꽃잎, 반딧불 등)
// - 빛 자리 효과 (횃불 불티 등). 같은 파일은 한 번만 읽어요.
// 잘못된 효과 파일은 이유를 기록하고 건너뛰어요. 그 효과만 안 보일 뿐 게임은 계속돼요.

#include "engine/fx/particle_effect.hpp"

#include "wind/world/ambience.hpp"
#include "wind/world/weather_table.hpp"

#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace wind::game {

struct FieldEffects {
    std::optional<engine::ParticleEffect> dust_walk;
    std::optional<engine::ParticleEffect> dust_dodge;
    std::vector<engine::ParticleEffect> ambient;  // 맵 분위기 효과 (+ 시험용 부하 효과)
    std::map<std::string, engine::ParticleEffect> light_fx;  // 빛 자리 효과 (파일 위치 -> 효과)
    std::vector<engine::ParticleEffect> night;    // 밤에만 나오는 효과 (반딧불 등)
    std::map<std::string, engine::ParticleEffect> weather_fx;  // 날씨 효과 (파일 위치 -> 효과)
};

// weather: 맵의 날씨 표 (없으면 nullptr). 날씨마다 쓰는 효과 파일도 같이 읽어요.
// stress_particles > 0 이면 성능 시험용으로 파티클이 약 그만큼 떠 있게 하는 효과를 더해요.
[[nodiscard]] FieldEffects load_field_effects(const std::filesystem::path& content_root,
                                              const world::Ambience& ambience,
                                              const world::WeatherTable* weather,
                                              std::uint32_t stress_particles);

}  // namespace wind::game
