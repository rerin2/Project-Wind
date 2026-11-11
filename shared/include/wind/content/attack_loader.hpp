#pragma once

// 이 파일은 무엇을 하나요?
// 공격 데이터 파일(content/attacks/*.json)을 읽고 숙제 검사를 하는 도우미예요. (명세 9.2)
//   { "schema_version": 1, "id": "longsword_1",
//     "windup": 0.08, "active": 0.10, "recovery": 0.18, "cancel_window": [0.18, 0.36],
//     "hitbox": { "offset": [18, -8], "size": [28, 24] },
//     "power": 1.0, "knockback": 90, "hitstop": 0.06, "shake": 0.25,
//     "next_combo": "longsword_2" }
// 선택 칸: "mp_cost", "fx", "sfx", "next_combo". 범위는 docs/ENGINE.md "Attack" 절에 있어요.

#include "wind/combat/attack.hpp"
#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kAttackSchemaVersion = 1;

[[nodiscard]] LoadResult<combat::AttackData> parse_attack(const nlohmann::json& document,
                                                          const std::string& file);

[[nodiscard]] LoadResult<combat::AttackData> load_attack_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::content
