#pragma once

// 이 파일은 무엇을 하나요?
// 무기 파일(content/weapons/*.json)을 읽고, 무기가 가리키는 공격 파일들까지 함께 읽어요. (명세 8.4)
//   { "schema_version": 1, "id": "longsword", "name": "롱소드",
//     "weight": 6, "base_power": 14, "strength_scaling": 0.8, "stamina_per_attack": 14,
//     "attacks": ["attacks/longsword_1.json", "attacks/longsword_2.json", "attacks/longsword_3.json"] }
// 선택 칸: "crit_bonus" (0~1).
// 콤보 검사: 공격 id는 겹치면 안 되고, 공격에 "next_combo"가 있으면 목록의 다음 공격 id와 같아야 해요.
// 마지막 공격에는 "next_combo"가 없어야 해요.

#include "wind/combat/attack.hpp"
#include "wind/combat/weapon.hpp"
#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kWeaponSchemaVersion = 1;

// 공격 파일 하나를 읽어 주는 함수. 진짜 게임에서는 load_attack_file, 테스트에서는 가짜를 넣어요.
using AttackSource = std::function<LoadResult<combat::AttackData>(std::string_view path)>;

[[nodiscard]] LoadResult<combat::Weapon> parse_weapon(const nlohmann::json& document,
                                                      const std::string& file,
                                                      const AttackSource& load_attack);

[[nodiscard]] LoadResult<combat::Weapon> load_weapon_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::content
