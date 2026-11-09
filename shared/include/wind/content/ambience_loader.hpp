#pragma once

// 이 파일은 무엇을 하나요?
// 맵 파일의 "ambience"(분위기) 묶음을 읽고 숙제 검사를 하는 도우미예요.
// 형식은 docs/ENGINE.md "Map" 절에 있어요. 틀린 칸은 "ambience.lights[2].radius"처럼
// 정확한 위치와 함께 오류 공책에 적어요.

#include "wind/content/error_list.hpp"
#include "wind/world/ambience.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <string>

namespace wind::content {

inline constexpr std::size_t kMaxAmbientFx = 8;   // 맵 하나에 주변 효과 최대 개수
inline constexpr std::size_t kMaxMapLights = 64;  // 맵 하나에 놓을 수 있는 빛 최대 개수

// object: ambience 묶음, map_width/map_height: 빛 위치가 맵 안인지 확인할 때 써요(칸 단위).
// 읽은 내용은 out에 채우고, 틀린 곳은 errors에 적어요.
void parse_ambience(const nlohmann::json& object, const std::string& file, float map_width,
                    float map_height, ErrorList& errors, world::Ambience& out);

}  // namespace wind::content
