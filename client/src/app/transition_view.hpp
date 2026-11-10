#pragma once

// 이 파일은 무엇을 하나요?
// 맵 옮기기 화면 그리기 도우미예요. (명세 6.8, 7.4)
// - 화면 가리기: 페이드는 화면 전체를 반투명 검정으로, 아이리스는 둥근 구멍 바깥을 새까맣게
// - 못 간 출구 알림: 화면 아래쪽 가운데에 검은 띠를 깔고 빨간 한글 글자로 보여 줘요

#include "engine/fx/screen_fade.hpp"
#include "engine/ui/ui_painter.hpp"

#include <string_view>

namespace wind::client {

// focus: 아이리스 구멍 가운데 (주인공 몸 가운데의 화면 위치)
void draw_transition(engine::Renderer& renderer, const engine::ScreenFade& fade, engine::Vec2 focus);
void draw_notice(engine::UiPainter& painter, std::string_view notice);

}  // namespace wind::client
