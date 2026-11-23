#pragma once

// 이 파일은 무엇을 하나요?
// 왼쪽 아래 아이템 알림 줄(LootToasts)을 그리는 도우미 함수예요. (14b)
// 줄마다 반투명 띠 + 16×16 아이콘 + 글자. 주운 것은 크림색, 가방이 가득 차면 붉은 주황 글자예요.
// 새 줄이 맨 아래이고, 위로 쌓여요. 왼쪽에서 미끄러져 들어오고 끝에서 흐려져요.

#include "engine/render/render_types.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/ui/loot_toasts.hpp"

namespace wind::client {

// icons: 아이템 아이콘 그림 판, screen: 화면 크기 (논리 해상도)
void draw_loot_toasts(engine::UiPainter& painter, engine::TextureHandle icons, engine::Vec2 screen,
                      const game::LootToasts& toasts);

}  // namespace wind::client
