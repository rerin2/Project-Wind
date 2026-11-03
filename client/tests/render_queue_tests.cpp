// 이 파일은 무엇을 하나요?
// 그림 순서표(렌더 큐)가 규칙대로 줄을 서는지 숙제 검사(테스트)를 해요. (명세 5.5)
// 규칙: 1) 그리는 단계(바닥이 먼저) -> 2) 발 위치(위쪽이 먼저) -> 3) 적은 순서
// 그림마다 번호(1, 2, 3...)를 붙여 두고, 줄 선 뒤 번호 순서가 맞는지 봐요.
#include "engine/render/render_queue.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace {

using wind::engine::DrawLayer;
using wind::engine::RenderQueue;
using wind::engine::SpriteDraw;

// 텍스처 번호를 표식으로 써서 정렬 뒤 순서를 확인한다.
SpriteDraw tagged(std::uint32_t tag) {
    SpriteDraw s;
    s.texture.id = tag;
    return s;
}

// 줄 선 순서대로 번호를 꺼내 목록으로 만들어요.
std::vector<std::uint32_t> order(const RenderQueue& queue) {
    std::vector<std::uint32_t> tags;
    for (const auto& cmd : queue.commands()) {
        tags.push_back(cmd.sprite.texture.id);
    }
    return tags;
}

// 나뭇잎(높은 물건)을 먼저 적어도, 바닥 -> 캐릭터 층 -> 나뭇잎 순서로 줄을 서야 해요.
TEST(RenderQueue, SortsByLayerFirst) {
    RenderQueue queue;
    queue.push_sprite(DrawLayer::HighObjects, 0.0f, tagged(3));
    queue.push_sprite(DrawLayer::Ground, 100.0f, tagged(1));
    queue.push_sprite(DrawLayer::YSorted, 50.0f, tagged(2));
    queue.sort();
    EXPECT_EQ(order(queue), (std::vector<std::uint32_t>{1, 2, 3}));
}

// 같은 층에서는 발이 화면 위쪽(작은 y)에 있는 것을 먼저 그려요. 그래서 뒤쪽에 깔려 보여요.
TEST(RenderQueue, SortsBySortYWithinLayer) {
    RenderQueue queue;
    queue.push_sprite(DrawLayer::YSorted, 96.0f, tagged(2));  // 아래쪽 캐릭터
    queue.push_sprite(DrawLayer::YSorted, 64.0f, tagged(1));  // 위쪽 나무 밑동
    queue.sort();
    EXPECT_EQ(order(queue), (std::vector<std::uint32_t>{1, 2}));
}

// 층도 발 위치도 똑같으면, 적은 순서 그대로여야 해요. 그래야 매번 똑같이 그려져요.
TEST(RenderQueue, KeepsInsertionOrderForTies) {
    RenderQueue queue;
    for (std::uint32_t tag = 1; tag <= 5; ++tag) {
        queue.push_sprite(DrawLayer::Ground, 0.0f, tagged(tag));
    }
    queue.sort();
    EXPECT_EQ(order(queue), (std::vector<std::uint32_t>{1, 2, 3, 4, 5}));
}

// clear()를 하면 순서표가 텅 비어야 해요.
TEST(RenderQueue, ClearResetsCommands) {
    RenderQueue queue;
    queue.push_fill(DrawLayer::DebugOverlay, 0.0f, {0, 0, 1, 1}, {});
    queue.clear();
    EXPECT_TRUE(queue.commands().empty());
}

}  // namespace
