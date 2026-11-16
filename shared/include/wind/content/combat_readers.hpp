#pragma once

// 이 파일은 무엇을 하나요?
// 공격 파일과 마술 파일이 함께 쓰는 "칸 읽기" 도우미 함수 모음이에요.
// - [a, b] 숫자 두 개 칸
// - "hitbox": { "offset": [앞, 위], "size": [길이, 폭] }  (명세 9.2)
// - "projectile": { "speed", "range", "size", "count", "homing", "orbit_seconds" }  (명세 9.2)
// 틀린 칸은 JsonReader의 오류 공책에 적고 nullopt를 돌려줘요. 범위는 docs/ENGINE.md에 있어요.

#include "wind/combat/attack.hpp"
#include "wind/combat/projectile_data.hpp"
#include "wind/content/error_list.hpp"
#include "wind/content/json_reader.hpp"

#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace wind::content {

// [a, b] 숫자 두 개짜리 칸. 둘 다 min~max여야 해요 (a <= b일 필요는 없어요).
[[nodiscard]] std::optional<std::array<double, 2>> read_number_pair(JsonReader& reader,
                                                                    std::string_view key,
                                                                    double min, double max);

// key 이름의 히트박스 묶음 (공격의 "hitbox", 마술의 "melee")
[[nodiscard]] std::optional<combat::HitboxShape> read_hitbox(JsonReader& parent,
                                                            std::string_view key,
                                                            const std::string& file,
                                                            ErrorList& errors);

// "projectile" 묶음
[[nodiscard]] std::optional<combat::ProjectileData> read_projectile(JsonReader& parent,
                                                                   const std::string& file,
                                                                   ErrorList& errors);

}  // namespace wind::content
