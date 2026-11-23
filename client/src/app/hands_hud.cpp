// 이 파일은 무엇을 하나요?
// HUD 왼쪽 아래 무기·마술 칸 그리기. 자리·색 값의 이유는 docs/VISUALS.md "Hands HUD" 절에 있어요.

#include "app/hands_hud.hpp"

#include "engine/render/renderer.hpp"
#include "engine/tween/easing.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/presentation/loot_view.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace wind::client {
namespace {

constexpr float kLeft = 8.0f;          // 화면 왼쪽에서 (알림 줄과 같은 줄)
constexpr float kBottom = 8.0f;        // 화면 아래에서
constexpr float kCell = 24.0f;         // 칸 크기 (아이콘 16 + 여백 4씩)
constexpr float kGap = 4.0f;           // 칸 사이
constexpr float kNameGap = 6.0f;       // 마술 칸과 이름 사이
constexpr float kNameWidth = 110.0f;   // 이름 자리 너비 (가장 긴 "큰 별빛 조각"이 들어가요)
constexpr float kPip = 4.0f;           // 마술 칸 점 크기
constexpr float kPipPitch = 7.0f;      // 점 간격
constexpr float kPopSeconds = 0.18f;   // Q로 바꿀 때 톡 커지는 시간 (명세 6.9: 0.15~0.25초)
constexpr float kPopScale = 0.3f;      // 톡: 처음에 1.3배
constexpr float kDimAlpha = 0.4f;      // MP가 모자랄 때 (인벤토리 못 고르는 항목과 같아요)
constexpr engine::Color kCellFill{0, 0, 0, 120};           // 칸 바탕 (알림 띠와 같은 어둡기)
constexpr engine::Color kCellBorder{90, 96, 112, 200};     // 칸 테두리 (흐린 회색)
constexpr engine::Color kPipEmpty{90, 96, 112, 200};       // 빈 마술 칸 점 테두리
constexpr engine::Color kPipFilled{154, 163, 181, 255};    // 마술이 든 칸 점 (흐린 회색, 테마 흐린 글자와 같아요)
constexpr engine::Color kPipSelected{255, 204, 92, 255};   // 고른 칸 점 (진한 금빛: 크림색 글자와 헷갈리지 않게)

// 1픽셀 테두리 (네 변)
void border(engine::Renderer& renderer, engine::RectF r, engine::Color color) {
    renderer.fill_rect({r.x, r.y, r.w, 1.0f}, color);
    renderer.fill_rect({r.x, r.y + r.h - 1.0f, r.w, 1.0f}, color);
    renderer.fill_rect({r.x, r.y, 1.0f, r.h}, color);
    renderer.fill_rect({r.x + r.w - 1.0f, r.y, 1.0f, r.h}, color);
}

}  // namespace

void HandsHud::update(float dt_seconds, HandsInfo info) {
    if (info.switch_count != info_.switch_count) pop_ = kPopSeconds;  // Q로 바뀌었어요
    pop_ = std::max(0.0f, pop_ - dt_seconds);
    info_ = std::move(info);
}

void HandsHud::draw_cell(engine::UiPainter& painter, engine::RectF cell, engine::TextureHandle sheet, int icon,
                         float scale, float alpha) const {
    auto& renderer = painter.renderer();
    renderer.fill_rect(cell, kCellFill);
    border(renderer, cell, kCellBorder);
    if (icon < 0) return;  // 맨손·마술 없음: 빈 칸
    constexpr int kIcon = game::LootView::kIconSize;
    const float size = static_cast<float>(kIcon) * scale;  // 톡: 가운데를 중심으로 커져요
    engine::SpriteDraw sprite;
    sprite.texture = sheet;
    sprite.source = {icon * kIcon, 0, kIcon, kIcon};
    sprite.dest = {std::round(cell.x + (cell.w - size) * 0.5f), std::round(cell.y + (cell.h - size) * 0.5f), size, size};
    sprite.tint = engine::with_alpha({255, 255, 255, 255}, alpha);
    renderer.draw_sprite(sprite);
}

void HandsHud::draw(engine::UiPainter& painter, engine::Vec2 screen) const {
    const auto& theme = painter.theme();
    const float y = screen.y - kBottom - kCell;
    // 1) 무기 칸
    draw_cell(painter, {kLeft, y, kCell, kCell}, item_icons_, info_.weapon_icon, 1.0f, 1.0f);
    // 2) 마술 칸: MP가 모자라면 흐리게, Q로 바꾼 직후면 톡 커졌다가 부드럽게 돌아와요
    const float spell_alpha = info_.spell_affordable ? 1.0f : kDimAlpha;
    const float pop = engine::ease(engine::Ease::OutCubic, pop_ / kPopSeconds);  // 1 → 0
    const engine::RectF spell_cell{kLeft + kCell + kGap, y, kCell, kCell};
    draw_cell(painter, spell_cell, spell_icons_, info_.spell_icon, 1.0f + kPopScale * pop, spell_alpha);
    // 3) 마술 이름 (위 줄)
    const float name_x = spell_cell.x + kCell + kNameGap;
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    engine::TextStyle style;
    style.color = info_.spell_icon < 0 ? theme.text_dim : theme.text;
    style.max_lines = 1;
    style.wrap = false;
    const std::string& name = info_.spell_icon < 0 ? std::string("마술 없음") : info_.spell_name;
    painter.text({name_x, y - 2.0f, kNameWidth, line_h}, name, style, spell_alpha);
    // 4) 마술 칸 점 (아래 줄): 고른 칸 진한 금빛, 마술 든 칸 흐린 회색, 빈 칸은 테두리만
    auto& renderer = painter.renderer();
    const float pip_y = y + kCell - kPip - 2.0f;
    for (std::size_t i = 0; i < info_.open_slots; ++i) {
        const engine::RectF pip{name_x + kPipPitch * static_cast<float>(i), pip_y, kPip, kPip};
        const bool filled = (info_.filled_mask & (1u << i)) != 0;
        if (!filled) {
            border(renderer, pip, kPipEmpty);
        } else {
            renderer.fill_rect(pip, i == info_.selected_slot ? kPipSelected : kPipFilled);
        }
    }
}

}  // namespace wind::client
