#pragma once

// 이 파일은 무엇을 하나요?
// 렌더 큐 = "그림 그릴 순서표"예요. (명세 5.5)
// 화면 한 장을 그리기 전에, 그릴 것을 일단 전부 순서표에 적어요.
// 그다음 정해진 규칙대로 줄을 세운 뒤 차례대로 그려요.
// 줄 세우는 규칙: 1) 그리는 단계(DrawLayer) -> 2) 발 위치(y) -> 3) 적은 순서
// 이렇게 규칙을 분명히 정해 두면, 매번 같은 순서로 그려져서 그림이 깜빡이거나 뒤바뀌지 않아요.

#include "engine/render/render_types.hpp"

#include <vector>

namespace wind::engine {

// 순서표의 한 줄 (그릴 것 하나)
struct DrawCommand {
    // 그릴 것의 종류: 그림 조각 / 색칠한 네모
    enum class Kind : std::uint8_t { Sprite, FillRect };

    DrawLayer layer = DrawLayer::Ground;
    float sort_y = 0.0f;         // 같은 단계 안에서 줄 세우는 기준 (맵 위 발 위치)
    std::uint32_t sequence = 0;  // 기준이 똑같을 때는 먼저 적은 것이 먼저
    Kind kind = Kind::Sprite;
    SpriteDraw sprite;  // 그림 조각일 때 쓰는 정보
    RectF rect;         // 색칠한 네모일 때: 화면의 어디에
    Color color;        // 색칠한 네모일 때: 무슨 색으로
};

class RenderQueue {
public:
    // 순서표를 깨끗이 비워요. (새 화면을 그리기 전에)
    void clear();
    // 그림 조각 하나를 순서표에 적어요.
    void push_sprite(DrawLayer layer, float sort_y, const SpriteDraw& sprite);
    // 색칠한 네모 하나를 순서표에 적어요.
    void push_fill(DrawLayer layer, float sort_y, RectF rect, Color color);

    // 규칙대로 줄을 세워요. 그리기 바로 전에 한 번 불러요.
    void sort();
    // 줄 선 순서표를 보여 줘요.
    [[nodiscard]] const std::vector<DrawCommand>& commands() const { return commands_; }

private:
    std::vector<DrawCommand> commands_;
    std::uint32_t next_sequence_ = 0;  // 다음에 적을 것의 번호 (적은 순서 기록용)
};

}  // namespace wind::engine
