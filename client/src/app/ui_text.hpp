#pragma once

// 이 파일은 무엇을 하나요?
// 메뉴 그리기에 자주 쓰는 작은 도우미 함수 모음이에요 (한 주제의 자유 함수, 15e).
// - 글자 한 줄 (한 줄로 자르기, 그림자)
// - 1픽셀 테두리
// - 16×16 아이콘 판에서 한 칸 그리기 (아이템·마술 아이콘)

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"
#include "engine/ui/ui_painter.hpp"

#include <string_view>

namespace wind::client {

// 아이콘 한 칸 크기 (아이템·마술 아이콘 판이 같아요)
inline constexpr float kUiIconSize = 16.0f;

// 글자 한 줄: 넘치면 잘려요 (잘림은 painter가 세어요)
void ui_line(engine::UiPainter& painter, engine::RectF box, std::string_view text, engine::Color color,
             float alpha = 1.0f, engine::TextAlign align = engine::TextAlign::Left,
             engine::FontStyle font = engine::FontStyle::Regular);
// 네모 둘레를 width픽셀 선으로
void ui_border(engine::Renderer& renderer, engine::RectF rect, engine::Color color, float width = 1.0f);
// 아이콘 판의 index번째 16×16 칸을 at(왼쪽 위)에 그려요
void ui_icon(engine::Renderer& renderer, engine::TextureHandle sheet, int index, engine::Vec2 at, float alpha);

}  // namespace wind::client
