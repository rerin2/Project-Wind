#pragma once

// 이 파일은 무엇을 하나요?
// 맵 하나의 바닥 아이템 목록이에요. (14b)
// - 아이템을 놓으면 새 번호를 붙여요 (번호는 다시 쓰지 않아요).
// - 너무 많이 쌓이지 않게 맵마다 64개까지. 넘으면 가장 오래된 것부터 사라져요.
// - 줍기 규칙은 FieldLoot가 하고, 이것은 목록만 가져요.
// 게임 규칙이라 SDL을 몰라요.

#include "game/items/ground_item.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace wind::game {

class MapLoot {
public:
    static constexpr std::size_t kMaxItems = 64;  // 맵 하나의 바닥 아이템 상한

    // 아이템을 놓고 붙인 번호를 돌려줘요. 상한을 넘으면 가장 오래된 것을 지워요.
    // wait_for_leave: 주인공이 내려놓은 것이면 true (멀어졌다 올 때까지 안 주워요, 14c)
    std::uint32_t drop(const items::ItemStack& stack, engine::Vec2 from, engine::Vec2 at, bool wait_for_leave = false);
    // 번호로 지워요 (주웠을 때). 없는 번호면 false
    bool remove(std::uint32_t id);
    // 모든 아이템의 나이를 dt만큼 늘려요
    void age(float dt_seconds);

    [[nodiscard]] const std::vector<GroundItem>& items() const { return items_; }
    [[nodiscard]] std::vector<GroundItem>& items() { return items_; }
    // 상한 때문에 사라진 아이템 수 (로그·F3용)
    [[nodiscard]] std::uint64_t discarded() const { return discarded_; }

private:
    std::vector<GroundItem> items_;  // 놓은 순서대로 (앞이 가장 오래됨)
    std::uint32_t next_id_ = 1;      // 다음에 붙일 번호
    std::uint64_t discarded_ = 0;
};

}  // namespace wind::game
