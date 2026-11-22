// 이 파일은 무엇을 하나요?
// 출구 칸 지켜보기: 무장 -> 밟으면 요청 한 번.

#include "game/world/exit_watcher.hpp"

#include "engine/map/coordinates.hpp"

#include <utility>

namespace wind::game {

void ExitWatcher::step(const world::MapData& map, engine::Vec2 feet, bool can_leave) {
    if (!can_leave) return;  // 쓰러졌거나 쉬는 중이면 떠나지 않아요
    const world::MapExit* exit = map.exit_at(engine::world_to_tile(feet, map.tile_size));
    if (exit == nullptr) {
        armed_ = true;                 // 출구 밖을 밟았으니 이제 출구가 동작해요
    } else if (armed_ && !request_) {
        request_ = *exit;              // 게임 루프에게: 이 출구로 옮겨 주세요
        armed_ = false;
    }
}

std::optional<world::MapExit> ExitWatcher::take_request() {
    return std::exchange(request_, std::nullopt);
}

}  // namespace wind::game
