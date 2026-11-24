#pragma once

// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 "마술 칸" 화면의 안쪽을 그려요 (15f). 창(패널)과 나타나기는 CushionMenuView가 해요.
// - 맨 위: "마술 칸" 제목과 지금 지능
// - 왼쪽: 칸 네 줄 ("칸 1" + 아이콘 + 마술 이름 / "비어 있음" / 잠긴 칸은 흐리게 "잠김"). 고른 칸은 금빛 바탕
// - 오른쪽 (2단계에서만): 배운 마술 목록 (아이콘, 이름, 요구 지능 — 모자라면 빨강, 다른 칸에 있으면 "칸 N")과 "비우기".
//   지능이 모자라 거절되면 목록이 좌우로 흔들려요.
// - 맨 아래: 키 안내

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

namespace wind::engine {
class UiPainter;
}
namespace wind::game {
class HeroProgress;
class SpellShelf;
class SpellSlotSheet;
}  // namespace wind::game

namespace wind::client {

struct SpellSlotDraw {
    engine::UiPainter& painter;
    engine::RectF content;                 // 창 안쪽 (여백 뺀 곳)
    float shake = 0.0f;                    // 거절 흔들림 (픽셀, 목록만 흔들려요)
    const game::SpellSlotSheet& sheet;
    const game::HeroProgress& progress;
    const game::SpellShelf* shelf;         // 마술 이름·아이콘·요구 지능 (없으면 id만)
    engine::TextureHandle icons;           // 마술 아이콘 판
};

void draw_spell_slot_sheet(const SpellSlotDraw& draw);

}  // namespace wind::client
