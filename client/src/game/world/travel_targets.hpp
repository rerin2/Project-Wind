#pragma once

// 이 파일은 무엇을 하나요?
// "어디로 옮길지"를 정하는 도우미 함수 모음이에요. (12c 부활, 13b 쉬기)
// 결과는 출구(MapExit) 모양이라 MapTravel이 보통 출구처럼 다뤄요.
// 쿠션 id는 맵의 등장 자리처럼 쓰여요: 필드 준비물이 등장 자리에서 못 찾으면 쿠션에서 찾아요.

#include "game/progression/hero_progress.hpp"

#include "wind/world/map.hpp"

#include <string_view>

namespace wind::game {

// 맵에 쿠션이 하나도 없거나 쉰 적이 없을 때 부활하는 등장 자리 (12c)
inline constexpr std::string_view kDefaultSpawn = "default";

// 부활할 곳: 마지막으로 쉰 쿠션, 없으면 지금 맵의 기본 자리 (명세 8.7, 9.5). 화면은 페이드예요.
[[nodiscard]] world::MapExit respawn_target(const HeroProgress& progress, std::string_view current_map);
// 쉬러 갈 곳: 이 맵의 이 쿠션 (같은 맵을 새로 읽어요). 화면은 페이드예요.
[[nodiscard]] world::MapExit cushion_target(std::string_view map, std::string_view cushion_id);

}  // namespace wind::game
