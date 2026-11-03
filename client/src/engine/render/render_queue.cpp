// 이 파일은 무엇을 하나요?
// 그림 순서표(렌더 큐)에 적고, 줄 세우는 방법을 실제로 적은 곳이에요.

#include "engine/render/render_queue.hpp"

#include <algorithm>
#include <tuple>

namespace wind::engine {

void RenderQueue::clear() {
    // 내용만 지우고 공간은 남겨 둬요. 매 장마다 새 공책을 사는 낭비를 막아요.
    commands_.clear();
    next_sequence_ = 0;
}

// 그림 조각을 순서표 맨 끝에 적어요. 번호표(sequence)도 하나씩 붙여요.
void RenderQueue::push_sprite(DrawLayer layer, float sort_y, const SpriteDraw& sprite) {
    DrawCommand cmd;
    cmd.layer = layer;
    cmd.sort_y = sort_y;
    cmd.sequence = next_sequence_++;
    cmd.kind = DrawCommand::Kind::Sprite;
    cmd.sprite = sprite;
    commands_.push_back(cmd);
}

// 색칠한 네모를 순서표 맨 끝에 적어요.
void RenderQueue::push_fill(DrawLayer layer, float sort_y, RectF rect, Color color) {
    DrawCommand cmd;
    cmd.layer = layer;
    cmd.sort_y = sort_y;
    cmd.sequence = next_sequence_++;
    cmd.kind = DrawCommand::Kind::FillRect;
    cmd.rect = rect;
    cmd.color = color;
    commands_.push_back(cmd);
}

void RenderQueue::sort() {
    // 단계 -> 발 위치 -> 번호표 순서로 비교해요. (tie = 여러 값을 한꺼번에 차례로 비교하기)
    std::ranges::sort(commands_, [](const DrawCommand& a, const DrawCommand& b) {
        return std::tie(a.layer, a.sort_y, a.sequence) < std::tie(b.layer, b.sort_y, b.sequence);
    });
}

}  // namespace wind::engine
