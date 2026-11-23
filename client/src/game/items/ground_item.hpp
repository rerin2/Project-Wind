#pragma once

// 이 파일은 무엇을 하나요?
// 바닥에 떨어진 아이템 하나예요. (명세 9.7, 14b)
// 맵마다 목록(MapLoot)에 들어 있고, 맵을 떠나도 그대로 남아요 (개발자와 정함).
// id는 그 맵 안에서 하나뿐이라, 한 번 주운 아이템은 다시 주워질 수 없어요.

#include "engine/core/math.hpp"

#include "wind/items/item_stack.hpp"

#include <cstdint>

namespace wind::game {

struct GroundItem {
    std::uint32_t id = 0;     // 이 맵 안에서 하나뿐인 번호 (1부터)
    items::ItemStack stack;   // 무엇이 몇 개
    engine::Vec2 from;        // 튀어나온 자리 (쓰러진 몬스터 발, 연출용)
    engine::Vec2 at;          // 떨어진 자리 (맵 위 픽셀, 줍기 거리를 여기서 재요)
    float age = 0.0f;         // 떨어진 뒤 흐른 게임 시간 (초). 튀어 오르는 동안은 못 주워요
    bool full_notice = false; // 가방이 가득 차서 "가득 찼다"를 이미 알렸나 (멀어지면 다시 false)
    bool wait_for_leave = false; // 주인공이 내려놓은 것: 48픽셀 밖으로 멀어졌다 올 때까지 안 주워요 (14c)
};

}  // namespace wind::game
