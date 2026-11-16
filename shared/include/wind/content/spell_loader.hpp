#pragma once

// 이 파일은 무엇을 하나요?
// 마술 파일(content/spells/*.json)을 읽고 숙제 검사를 하는 도우미예요. (명세 8.6, 14.2)
//   { "schema_version": 1, "id": "star_shard", "name": "별빛 조각",
//     "intelligence_required": 10, "mp_cost": 6, "cast_seconds": 0.25, "power": 1.0,
//     "projectile": { "speed": 360, "range": 240, "size": [6, 6] },
//     "light": { "radius": 36, "color": "#9cc8ff", "intensity": 0.8 } }
// 검사 규칙:
// - "mp_cost"와 "mp_per_second" 중 꼭 하나만 (mp_per_second는 광선만, 광선은 mp_per_second만)
// - 동작 "projectile" / "melee" / "beam" 중 꼭 하나만
// - 선택 칸: "fx", "sfx", "light"
// 범위는 docs/ENGINE.md "Spell" 절에 있어요.

#include "wind/combat/spell.hpp"
#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kSpellSchemaVersion = 1;

[[nodiscard]] LoadResult<combat::Spell> parse_spell(const nlohmann::json& document,
                                                    const std::string& file);

[[nodiscard]] LoadResult<combat::Spell> load_spell_file(const std::filesystem::path& content_root,
                                                        std::string_view path);

}  // namespace wind::content
