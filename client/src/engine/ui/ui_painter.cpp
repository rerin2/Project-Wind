// 이 파일은 무엇을 하나요?
// 창·버튼·글을 실제로 그리는 곳이에요. 그리기는 Renderer와 TextRenderer에게 맡겨요.

#include "engine/ui/ui_painter.hpp"

#include "engine/ui/nine_slice.hpp"

#include "wind/log.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {
namespace {

// 버튼 글자는 버튼 세로 가운데에 와요. 글꼴 위쪽 여백 때문에 1픽셀 올려서 맞춰요.
constexpr float kButtonTextNudge = -1.0f;
// 글 상자 안쪽 여백 없이, 그림자는 1픽셀 아래
constexpr float kShadowOffset = 1.0f;
// 그림이 없을 때 쓰는 단색들
constexpr Color kFallbackPanel{28, 32, 50, 255};
constexpr Color kFallbackButton{44, 50, 74, 255};
constexpr Color kFallbackFocused{64, 72, 106, 255};

}  // namespace

Color with_alpha(Color color, float alpha) {
    const float a = std::clamp(alpha, 0.0f, 1.0f) * static_cast<float>(color.a);
    color.a = static_cast<std::uint8_t>(std::lround(a));
    return color;
}

UiPainter::UiPainter(Renderer& renderer, TextRenderer& text, const UiTheme& theme, TextureHandle frames)
    : renderer_(renderer), text_(text), theme_(theme), frames_(frames) {}

void UiPainter::frame(const UiFrame& frame, RectF rect, float alpha, Color tint) {
    for (const auto& piece : nine_slice(frame.source, frame.border, rect)) {
        if (piece.dest.w <= 0.0f || piece.dest.h <= 0.0f || piece.source.w <= 0 || piece.source.h <= 0) {
            continue;
        }
        SpriteDraw draw;
        draw.texture = frames_;
        draw.source = piece.source;
        draw.dest = piece.dest;
        draw.tint = with_alpha(tint, alpha);
        renderer_.draw_sprite(draw);
    }
}

void UiPainter::panel(RectF rect, float alpha) {
    const float a = alpha * static_cast<float>(theme_.panel_alpha) / 255.0f;
    if (!frames_.valid()) {
        renderer_.fill_rect(rect, with_alpha(kFallbackPanel, a));
        return;
    }
    frame(theme_.panel, rect, a);
}

void UiPainter::button(RectF rect, std::string_view label, bool focused, float alpha) {
    if (frames_.valid()) {
        frame(focused ? theme_.button_focused : theme_.button, rect, alpha);
    } else {
        renderer_.fill_rect(rect, with_alpha(focused ? kFallbackFocused : kFallbackButton, alpha));
    }
    TextStyle style;
    style.color = focused ? theme_.text_focused : theme_.text;
    style.align = TextAlign::Center;
    style.wrap = false;
    style.max_lines = 1;
    const float height = line_height(style.font);
    const float y = rect.y + std::floor((rect.h - height) * 0.5f) + kButtonTextNudge;
    const float pad = static_cast<float>(theme_.button.border);
    text({rect.x + pad, y, std::max(0.0f, rect.w - pad * 2.0f), height}, label, style, alpha);
}

TextLayout UiPainter::text(RectF box, std::string_view text, const TextStyle& style, float alpha) {
    TextLayoutOptions options;
    options.max_width = box.w;
    options.max_lines = style.wrap ? style.max_lines : 1;
    options.line_height = line_height(style.font);
    options.align = style.align;
    // 줄바꿈하지 않는 상자는 줄을 넘기지 않도록 줄바꿈 문자 없이 한 줄로만 놓아요.
    TextLayout layout = layout_text(text, text_.measurer(style.font), options);

    // 상자 높이를 넘는 줄은 그리지 않고, 잘림으로 셉니다.
    if (box.h > 0.0f && options.line_height > 0.0f) {
        const auto fit = static_cast<std::size_t>(std::max(1.0f, std::floor(box.h / options.line_height)));
        if (layout.lines.size() > fit) {
            layout.lines.resize(fit);
            layout.truncated = true;
        }
    }
    if (layout.truncated) {
        ++clipped_;
        if (warned_.insert(std::string(text)).second) {
            log::warn("UI text clipped (box {:.0f}x{:.0f}): {}", box.w, box.h, text);
        }
    }
    for (const auto& line : layout.lines) {
        const Vec2 at{box.x + line.x, box.y + line.y};
        if (style.shadow) {
            text_.draw(style.font, {at.x, at.y + kShadowOffset}, line.text, with_alpha(theme_.text_shadow, alpha));
        }
        text_.draw(style.font, at, line.text, with_alpha(style.color, alpha));
    }
    return layout;
}

float UiPainter::line_height(FontStyle font) const {
    return text_.line_height(font);
}

float UiPainter::measure(FontStyle font, std::string_view text) const {
    return text_.measure(font, text);
}

}  // namespace wind::engine
