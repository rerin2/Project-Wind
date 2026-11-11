#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면 하나를 만들 때 "파일에서 읽어 오는 준비물"을 한꺼번에 챙기는 도우미예요.
// - 맵(+ 타일셋), 주인공 그림 설명서, 등장 자리
// - 하루 색 곡선, 날씨 표 (바깥 맵)
// - 파티클 효과들 (먼지, 분위기, 밤, 날씨, 빛 자리)
// 꼭 필요한 것(맵, 그림 설명서, 등장 자리)이 잘못되면 이유를 기록하고 "없음"을 돌려줘요.
// 있으면 좋은 것(효과, 하루 색, 날씨)이 잘못되면 기록만 하고 빼고 계속해요.

#include "engine/animation/sprite_sheet.hpp"
#include "game/presentation/field_effects.hpp"

#include "wind/combat/weapon.hpp"
#include "wind/content/map_loader.hpp"
#include "wind/world/day_cycle.hpp"
#include "wind/world/direction.hpp"
#include "wind/world/weather_table.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string_view>

namespace wind::game {

struct FieldAssets {
    content::LoadedMap map;
    engine::SpriteSheet sheet;
    world::TileCoord spawn_tile;
    FieldEffects effects;
    std::optional<world::DayCycle> day_cycle;
    std::optional<world::WeatherTable> weather;
    combat::Weapon weapon;  // 든 무기 (불러오기 실패면 콤보가 빈 맨손)
};

// spawn_id: 등장 자리 이름. "default"인데 없으면 첫 번째 자리를 써요.
// stress_particles: 성능 시험용 파티클 수 (보통 0)
[[nodiscard]] std::optional<FieldAssets> load_field_assets(const std::filesystem::path& content_root,
                                                           std::string_view map_path,
                                                           std::string_view sheet_path,
                                                           std::string_view spawn_id,
                                                           std::uint32_t stress_particles);

}  // namespace wind::game
