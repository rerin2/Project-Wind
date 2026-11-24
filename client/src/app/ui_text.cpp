// 이 파일은 무엇을 하나요?
// 메뉴 그리기 도우미: 글자 한 줄, 테두리, 아이콘 한 칸.

#include "app/ui_text.hpp"

#include "engine/render/renderer.hpp"

namespace wind::client {

void ui_line(engine::UiPainter& painter, engine::RectF box, std::string_view text, engine::Color color, float alpha,
             engine::TextAlign align, engine::FontStyle font) {
    engine::TextStyle style;
    style.font = font;
    style.color = color;
    style.align = align;
    style.max_lines = 1;  // 한 줄만
    style.wrap = false;   // 넘치면 잘려요
    painter.text(box, text, style, alpha);
}

void ui_border(engine::Renderer& renderer, engine::RectF r, engine::Color color, float width) {
    renderer.fill_rect({r.x, r.y, r.w, width}, color);                // 위
    renderer.fill_rect({r.x, r.y + r.h - width, r.w, width}, color);  // 아래
    renderer.fill_rect({r.x, r.y, width, r.h}, color);                // 왼쪽
    renderer.fill_rect({r.x + r.w - width, r.y, width, r.h}, color);  // 오른쪽
}

void ui_icon(engine::Renderer& renderer, engine::TextureHandle sheet, int index, engine::Vec2 at, float alpha) {
    if (!sheet.valid() || index < 0) return;  // 그림 판이 없으면 안 그려요
    const int size = static_cast<int>(kUiIconSize);
    engine::SpriteDraw sprite;
    sprite.texture = sheet;
    sprite.source = {index * size, 0, size, size};  // 판에서 index번째 칸
    sprite.dest = {at.x, at.y, kUiIconSize, kUiIconSize};
    sprite.tint = engine::with_alpha({255, 255, 255, 255}, alpha);
    renderer.draw_sprite(sprite);
}

}  // namespace wind::client
