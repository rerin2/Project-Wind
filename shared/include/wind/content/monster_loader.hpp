#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터 설명 파일(content/monsters/*.json)을 읽고 숙제 검사를 하는 도우미예요. (11a)
//   { "schema_version": 1, "id": "slime_green", "name": "초록 슬라임",
//     "max_hp": 30, "attack_power": 4, "defense": 0, "move_speed": 40,
//     "detect_range": 120, "attack_range": 40, "leash_range": 260, "notice_seconds": 0.4,
//     "wander": { "radius": 40, "speed": 18, "pause": [1.0, 2.5] },
//     "body": [12, 6],
//     "hurtbox": { "offset": [-9, -13], "size": [18, 13] },
//     "sprite": { "image": "monsters/slime_green.bmp", "frame": [24, 24], "origin": [12, 21] },
//     "attacks": ["attacks/slime_hop.json"] }
// 숫자가 범위를 벗어나거나, 거리 순서(공격 <= 감지 <= 추적 한계, 배회 반지름 + 감지 <= 추적 한계)가 틀리거나,
// 공격 파일(1~6개)을 못 읽거나,
// 그림 위치가 콘텐츠 폴더 밖이면 오류 목록을 돌려줘요. 범위는 docs/ENGINE.md "Monster" 절에 있어요.

#include "wind/combat/monster_data.hpp"
#include "wind/content/attack_loader.hpp"
#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace wind::content {

inline constexpr std::int64_t kMonsterSchemaVersion = 1;

// load_attack: 공격 파일 하나를 읽는 함수 (테스트에서는 가짜를 넣어요)
[[nodiscard]] LoadResult<combat::MonsterData> parse_monster(const nlohmann::json& document,
                                                            const std::string& file,
                                                            const AttackSource& load_attack);

[[nodiscard]] LoadResult<combat::MonsterData> load_monster_file(const std::filesystem::path& content_root,
                                                                std::string_view path);

}  // namespace wind::content
