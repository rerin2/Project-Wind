#pragma once

// 이 파일은 무엇을 하나요?
// UI 화가예요. 창(패널), 글(라벨), 버튼을 테마(theme.json) 모양대로 그려요. (명세 6.9, 13.3)
// - 창과 버튼은 9조각 테두리로 그려서 어떤 크기든 테두리가 또렷해요.
// - 글은 text_layout으로 줄바꿈·정렬·잘림을 정한 다음, 1픽셀 그림자와 함께 써요.
// - 글자 잘림 검사: 상자에 다 안 들어가서 "..."으로 잘린 글을 세고, 처음 한 번은 경고로 기록해요.
//   (디버그 화면에 이번 화면의 잘린 글 수를 보여 줘요.)
// 테두리 그림이 없으면 단색 네모로 대신 그려서, 그림이 빠져도 UI는 보여요.

#include "engine/render/renderer.hpp"
#include "engine/render/text_renderer.hpp"
#include "engine/text/text_layout.hpp"
#include "engine/ui/ui_theme.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <string_view>

namespace wind::engine {

// 글 상자 하나를 그리는 방법
struct TextStyle {
    FontStyle font = FontStyle::Regular;
    Color color{255, 255, 255, 255};
    TextAlign align = TextAlign::Left;
    std::size_t max_lines = 0;  // 0이면 제한 없음
    bool wrap = true;           // false면 줄바꿈 없이 한 줄 (넘치면 잘림)
    bool shadow = true;         // 1픽셀 아래 그림자
};

class UiPainter {
public:
    // frames: 테두리 그림 판 (없으면 단색으로 대신 그려요)
    UiPainter(Renderer& renderer, TextRenderer& text, const UiTheme& theme, TextureHandle frames);

    // 화면 한 장을 시작할 때: 잘린 글 수를 0으로
    void begin_frame() { clipped_ = 0; }

    // 9조각 테두리 하나 그리기. alpha: 0~1
    void frame(const UiFrame& frame, RectF rect, float alpha = 1.0f, Color tint = {255, 255, 255, 255});
    // 창
    void panel(RectF rect, float alpha = 1.0f);
    // 버튼: 고른 버튼은 금색 테두리와 밝은 글자
    void button(RectF rect, std::string_view label, bool focused, float alpha = 1.0f);
    // 글 상자: box 안에 글을 놓아요. 놓은 결과(잘렸는지 등)를 돌려줘요.
    TextLayout text(RectF box, std::string_view text, const TextStyle& style, float alpha = 1.0f);

    [[nodiscard]] float line_height(FontStyle font) const;
    [[nodiscard]] float measure(FontStyle font, std::string_view text) const;
    [[nodiscard]] const UiTheme& theme() const { return theme_; }
    [[nodiscard]] Renderer& renderer() { return renderer_; }
    // 이번 화면에서 잘린 글 수 (글자 잘림 검사)
    [[nodiscard]] std::uint32_t clipped_this_frame() const { return clipped_; }

private:
    Renderer& renderer_;
    TextRenderer& text_;
    const UiTheme& theme_;
    TextureHandle frames_;
    std::uint32_t clipped_ = 0;
    std::set<std::string> warned_;  // 이미 경고한 잘린 글 (같은 경고를 계속 쓰지 않게)
};

// 투명도(0~1)를 곱한 색
[[nodiscard]] Color with_alpha(Color color, float alpha);

}  // namespace wind::engine
