#pragma once

// 이 파일은 무엇을 하나요?
// HUD 왼쪽 아래 "손" 칸이에요. (명세 13.2, 15b)
// - 무기 칸: 지금 끼고 있는 무기의 아이템 아이콘 (맨손이면 빈 칸)
// - 마술 칸: 고른 마술 아이콘, 오른쪽에 마술 이름, 그 아래 작은 점으로 마술 칸들(고른 칸은 금빛, 빈 칸은 테두리만)
//   MP가 모자라면 마술 아이콘과 이름이 흐려져요. Q로 바꾸면 아이콘이 톡 커졌다 돌아와요.
// 줍기 알림 줄은 이 칸들 위에 떠요 (개발자와 정함: 아이콘이 아래, 알림은 위).
// 게임 규칙은 바꾸지 않고 보여 주기만 해요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace wind::engine {
class UiPainter;
}

namespace wind::client {

// 마술 아이콘 그림 판 (16×16 칸, 마술 데이터 "icon" 번호 순서, 생성 스크립트 "마술 아이콘" 절, 15b)
inline constexpr std::string_view kSpellIconsTexture = "ui/spell_icons.bmp";

// 손 칸이 보여 줄 것 (ItemHud가 장비·마술 공책·필드에서 모아 줘요)
struct HandsInfo {
    int weapon_icon = -1;        // 무기 아이템 아이콘 칸 (-1 = 맨손)
    int spell_icon = -1;         // 고른 마술 아이콘 칸 (-1 = 마술 없음)
    std::string spell_name;      // 고른 마술 이름
    bool spell_affordable = true;  // MP가 충분한가 (모자라면 흐리게)
    std::size_t open_slots = 0;  // 열린 마술 칸 수 (지능, 2~4)
    unsigned filled_mask = 0;    // 마술이 든 칸 (1번 칸 = 1, 2번 칸 = 2, 3번 칸 = 4 ...)
    std::size_t selected_slot = 0;  // 고른 칸
    unsigned switch_count = 0;   // Q로 바꾼 횟수 (늘면 아이콘이 톡)
};

class HandsHud {
public:
    // 알림 줄을 이만큼 위로 올려요 (칸 높이 + 틈)
    static constexpr float kReservedHeight = 32.0f;

    // item_icons: 아이템 아이콘 그림 판 (무기), spell_icons: 마술 아이콘 그림 판
    HandsHud(engine::TextureHandle item_icons, engine::TextureHandle spell_icons)
        : item_icons_(item_icons), spell_icons_(spell_icons) {}

    // 화면 한 장마다: 보여 줄 것을 받고, 바꾼 횟수가 늘었으면 톡 시계를 다시 시작해요
    void update(float dt_seconds, HandsInfo info);
    void draw(engine::UiPainter& painter, engine::Vec2 screen) const;

private:
    // 칸 하나: 어두운 바탕과 테두리, 아이콘 (scale: 톡 커지기, alpha: 흐리기)
    void draw_cell(engine::UiPainter& painter, engine::RectF cell, engine::TextureHandle sheet, int icon,
                   float scale, float alpha) const;

    engine::TextureHandle item_icons_;
    engine::TextureHandle spell_icons_;
    HandsInfo info_;
    float pop_ = 0.0f;  // 톡 남은 시간 (초)
};

}  // namespace wind::client
