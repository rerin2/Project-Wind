// 이 파일은 무엇을 하나요?
// 부활·쉬기로 옮길 곳을 출구 모양으로 만들어요.

#include "game/world/travel_targets.hpp"

#include <string>

namespace wind::game {

world::MapExit respawn_target(const HeroProgress& progress, std::string_view current_map) {
    if (const auto& cushion = progress.last_cushion()) {  // 쉰 적이 있으면 그 쿠션 위에서 깨어나요
        return cushion_target(cushion->map, cushion->id);
    }
    world::MapExit home;                                  // 아직 없으면 지금 맵의 기본 자리 (12c 그대로)
    home.to_map = std::string(current_map);
    home.to_spawn = std::string(kDefaultSpawn);
    home.transition = world::ScreenTransition::Fade;
    return home;
}

world::MapExit cushion_target(std::string_view map, std::string_view cushion_id) {
    world::MapExit target;
    target.to_map = std::string(map);
    target.to_spawn = std::string(cushion_id);  // 쿠션 id = 등장 자리 이름
    target.transition = world::ScreenTransition::Fade;
    return target;
}

}  // namespace wind::game
