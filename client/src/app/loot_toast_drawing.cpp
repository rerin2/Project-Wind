// 이 파일은 무엇을 하나요?
// 아이템 알림 줄 그리기. 자리·색 값의 이유는 docs/VISUALS.md "Ground items" 절에 있어요.

#include "app/loot_toast_drawing.hpp"

#include "engine/render/renderer.hpp"
#include "game/presentation/loot_view.hpp"

#include <cmath>

namespace wind::client {
namespace {

constexpr float kLeft = 8.0f;          // 화면 왼쪽에서 띠까지
constexpr float kBottom = 10.0f;       // 화면 아래에서 맨 아래 줄 바닥까지
constexpr float kLineHeight = 20.0f;   // 줄 간격 (아이콘 16 + 여백)
constexpr float kSlideFrom = 40.0f;    // 이만큼 왼쪽에서 미끄러져 들어와요
constexpr float kPad = 3.0f;           // 띠 안쪽 여백
constexpr float kIconGap = 4.0f;       // 아이콘과 글자 사이
constexpr engine::Color kBand{0, 0, 0, 120};                 // 글자 뒤 반투명 띠
constexpr engine::Color kTextColor{240, 232, 200, 255};      // 크림색 (주움)
constexpr engine::Color kFullColor{240, 120, 96, 255};       // 붉은 주황 (가방이 가득 참)

}  // namespace

void draw_loot_toasts(engine::UiPainter& painter, engine::TextureHandle icons, engine::Vec2 screen,
                      const game::LootToasts& toasts, float lift) {
    const auto& lines = toasts.lines();
    const auto font = engine::FontStyle::Regular;
    const float text_h = painter.line_height(font);
    const float icon = static_cast<float>(game::LootView::kIconSize);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];
        const std::size_t from_bottom = lines.size() - 1 - i;  // 새 줄(맨 뒤)이 맨 아래
        const float alpha = game::LootToasts::alpha(line);
        const float x = std::round(kLeft - kSlideFrom * (1.0f - game::LootToasts::slide(line)));
        const float y = std::round(screen.y - kBottom - lift - icon - static_cast<float>(from_bottom) * kLineHeight);
        const float text_w = painter.measure(font, line.text);
        // 띠: 아이콘 + 글자를 감싸요
        painter.renderer().fill_rect({x - kPad, y - kPad, icon + kIconGap + text_w + kPad * 2.0f, icon + kPad * 2.0f},
                                     engine::with_alpha(kBand, alpha));
        // 아이콘: 그림 판에서 이 아이템 칸만 오려 붙여요
        engine::SpriteDraw sprite;
        sprite.texture = icons;
        sprite.source = {line.icon * game::LootView::kIconSize, 0, game::LootView::kIconSize, game::LootView::kIconSize};
        sprite.dest = {x, y, icon, icon};
        sprite.tint = engine::with_alpha({255, 255, 255, 255}, alpha);
        painter.renderer().draw_sprite(sprite);
        // 글자: 아이콘 오른쪽, 세로 가운데
        engine::TextStyle style;
        style.font = font;
        style.color = line.warning() ? kFullColor : kTextColor;
        style.max_lines = 1;
        style.wrap = false;
        const float text_y = std::round(y + (icon - text_h) * 0.5f);
        painter.text({x + icon + kIconGap, text_y, text_w + 1.0f, text_h}, line.text, style, alpha);
    }
}

}  // namespace wind::client
