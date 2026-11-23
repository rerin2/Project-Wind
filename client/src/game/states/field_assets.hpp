#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면 하나를 만들 때 "파일에서 읽어 오는 준비물"을 한꺼번에 챙기는 도우미예요.
// - 맵(+ 타일셋), 주인공 그림 설명서, 등장 자리
// - 하루 색 곡선, 날씨 표 (바깥 맵)
// - 파티클 효과들 (먼지, 분위기, 밤, 날씨, 빛 자리)
// - 맵의 허수아비들과 몬스터들 (11a), 캣닢 쿠션들 (13b)
// 꼭 필요한 것(맵, 그림 설명서, 등장 자리)이 잘못되면 이유를 기록하고 "없음"을 돌려줘요.
// 있으면 좋은 것(효과, 하루 색, 날씨)이 잘못되면 기록만 하고 빼고 계속해요.

#include "engine/animation/sprite_sheet.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/monsters/monster.hpp"
#include "game/presentation/field_effects.hpp"
#include "game/world/cushion.hpp"

#include "wind/combat/spell.hpp"
#include "wind/content/map_loader.hpp"
#include "wind/world/day_cycle.hpp"
#include "wind/world/direction.hpp"
#include "wind/world/weather_table.hpp"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::game {

struct FieldAssets {
    content::LoadedMap map;
    engine::SpriteSheet sheet;
    world::TileCoord spawn_tile;
    FieldEffects effects;
    std::optional<world::DayCycle> day_cycle;
    std::optional<world::WeatherTable> weather;
    std::vector<TrainingDummy> dummies;  // 맵에 놓인 훈련용 허수아비들 (9c)
    std::optional<combat::Spell> spell;  // 고른 마술 (9e: 별빛 조각 고정, 실패면 마술 없음)
    std::vector<Monster> monsters;       // 맵의 몬스터들 (11a, 설명 파일이 잘못된 몬스터는 빠져요)
    std::vector<Cushion> cushions;       // 맵의 캣닢 쿠션들 (13b)
    std::optional<std::string> start_cushion;  // 쿠션 위에서 시작하면 그 쿠션 id (쉬기·부활, 13b)
};

// 맵에 들어갈 때 무엇을 읽었는지 기록해요 (맵 크기, 효과·빛 수, 허수아비·몬스터 수).
void log_field_entry(const FieldAssets& assets);

// spawn_id: 등장 자리 이름 또는 쿠션 id (13b). "default"인데 없으면 첫 번째 자리를 써요.
// stress_particles: 성능 시험용 파티클 수 (보통 0)
[[nodiscard]] std::optional<FieldAssets> load_field_assets(const std::filesystem::path& content_root,
                                                           std::string_view map_path,
                                                           std::string_view sheet_path,
                                                           std::string_view spawn_id,
                                                           std::uint32_t stress_particles);

}  // namespace wind::game
