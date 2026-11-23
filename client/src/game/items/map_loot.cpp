// 이 파일은 무엇을 하나요?
// 맵 하나의 바닥 아이템 목록: 놓기(번호 붙이기, 상한), 지우기, 나이 먹기.

#include "game/items/map_loot.hpp"

#include "wind/log.hpp"

#include <algorithm>

namespace wind::game {

std::uint32_t MapLoot::drop(const items::ItemStack& stack, engine::Vec2 from, engine::Vec2 at) {
    if (items_.size() >= kMaxItems) {
        // 가장 오래된 것(맨 앞)을 지워 자리를 만들어요
        log::info("Ground item limit reached; removing oldest {} x{}", items_.front().stack.item_id,
                  items_.front().stack.count);
        items_.erase(items_.begin());
        ++discarded_;
    }
    const std::uint32_t id = next_id_++;
    items_.push_back({id, stack, from, at});
    return id;
}

bool MapLoot::remove(std::uint32_t id) {
    return std::erase_if(items_, [id](const GroundItem& item) { return item.id == id; }) > 0;
}

void MapLoot::age(float dt_seconds) {
    for (auto& item : items_) item.age += dt_seconds;
}

}  // namespace wind::game
