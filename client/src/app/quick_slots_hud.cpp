// 이 파일은 무엇을 하나요?
// 아래 가운데 단축 칸 1~4 그리기와 "썼을 때 톡". 자리·색 값의 이유는 docs/VISUALS.md "Quick slots" 절에 있어요.

#include "app/quick_slots_hud.hpp"

#include "app/ui_text.hpp"
#include "engine/render/renderer.hpp"
#include "engine/tween/easing.hpp"
#include "engine/ui/ui_painter.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace wind::client {
namespace {

constexpr float kBottom = 8.0f;        // 화면 아래에서 (왼쪽 아래 무기·마술 칸과 같은 줄)
constexpr float kCell = 24.0f;         // 칸 크기 (무기·마술 칸과 같아요)
constexpr float kGap = 4.0f;           // 칸 사이
constexpr float kPopSeconds = 0.18f;   // 썼을 때 톡 (명세 6.9: 0.15~0.25초, 마술 칸 톡과 같아요)
constexpr float kPopScale = 0.25f;     // 톡: 처음에 1.25배
constexpr float kDimAlpha = 0.4f;      // 0개·빈 칸 (흐린 항목과 같아요)
constexpr engine::Color kCellFill{0, 0, 0, 120};          // 칸 바탕 (무기·마술 칸과 같아요)
constexpr engine::Color kCellBorder{90, 96, 112, 200};    // 칸 테두리
constexpr engine::Color kNumberColor{255, 204, 92, 255};  // 칸 번호 (진한 금빛, 가방 칸 번호와 같아요)
constexpr engine::Color kEmptyCount{236, 96, 96, 255};    // 0개 (빨강)
constexpr engine::Color kCountBack{0, 0, 0, 150};         // 숫자 뒤 작은 띠

}  // namespace

void QuickSlotsHud::update(float dt_seconds, const std::array<QuickSlotInfo, kCount>& info) {
    for (std::size_t i = 0; i < kCount; ++i) {
        const auto& next = info[i];
        // 같은 아이템이 그대로 등록돼 있는데 개수가 줄었으면 썼어요: 톡
        const bool same_item = next.assigned && info_[i].assigned && next.icon == info_[i].icon;
        if (same_item && seen_count_[i] && next.count < *seen_count_[i]) pop_[i] = kPopSeconds;
        seen_count_[i] = next.assigned ? std::optional<std::int64_t>{next.count} : std::nullopt;
        pop_[i] = std::max(0.0f, pop_[i] - dt_seconds);
    }
    info_ = info;
}

void QuickSlotsHud::draw(engine::UiPainter& painter, engine::Vec2 screen) const {
    auto& renderer = painter.renderer();
    const float line_h = painter.line_height(engine::FontStyle::Regular);
    const float total = kCell * static_cast<float>(kCount) + kGap * static_cast<float>(kCount - 1);
    const float left = std::round((screen.x - total) * 0.5f);  // 화면 가운데
    const float top = screen.y - kBottom - kCell;
    for (std::size_t i = 0; i < kCount; ++i) {
        const auto& slot = info_[i];
        // 톡: 1.25배에서 OutCubic으로 돌아와요 (가운데를 중심으로)
        const float t = 1.0f - pop_[i] / kPopSeconds;
        const float scale = 1.0f + kPopScale * (1.0f - engine::ease(engine::Ease::OutCubic, std::clamp(t, 0.0f, 1.0f)));
        const float size = std::round(kCell * scale);
        const float cx = left + (kCell + kGap) * static_cast<float>(i) + kCell * 0.5f;
        const engine::RectF cell{std::round(cx - size * 0.5f), std::round(top + kCell * 0.5f - size * 0.5f), size, size};
        const float alpha = slot.assigned ? 1.0f : kDimAlpha;
        renderer.fill_rect(cell, engine::with_alpha(kCellFill, alpha));
        ui_border(renderer, cell, engine::with_alpha(kCellBorder, alpha));
        if (slot.assigned) {  // 아이콘 (0개면 흐리게)
            const float icon = kUiIconSize * scale;
            engine::SpriteDraw sprite;
            sprite.texture = icons_;
            sprite.source = {slot.icon * static_cast<int>(kUiIconSize), 0, static_cast<int>(kUiIconSize),
                             static_cast<int>(kUiIconSize)};
            sprite.dest = {std::round(cx - icon * 0.5f), std::round(cell.y + (size - icon) * 0.5f), icon, icon};
            sprite.tint = engine::with_alpha({255, 255, 255, 255}, slot.count > 0 ? 1.0f : kDimAlpha);
            if (icons_.valid()) renderer.draw_sprite(sprite);
            // 개수: 오른쪽 아래 (0개는 빨강)
            const std::string count = std::to_string(slot.count);
            const float count_w = painter.measure(engine::FontStyle::Regular, count);
            const float count_y = cell.y + cell.h - line_h + 2.0f;
            renderer.fill_rect({cell.x + cell.w - count_w - 2.0f, count_y + 2.0f, count_w + 1.0f, line_h - 3.0f}, kCountBack);
            ui_line(painter, {cell.x, count_y, cell.w - 1.0f, line_h}, count,
                    slot.count > 0 ? painter.theme().text : kEmptyCount, 1.0f, engine::TextAlign::Right);
        }
        // 번호: 왼쪽 위 (1~4)
        const std::string number = std::to_string(i + 1);
        const float number_w = painter.measure(engine::FontStyle::Regular, number);
        renderer.fill_rect({cell.x + 1.0f, cell.y + 1.0f, number_w + 2.0f, line_h - 4.0f}, engine::with_alpha(kCountBack, alpha));
        ui_line(painter, {cell.x + 2.0f, cell.y - 1.0f, number_w + 2.0f, line_h}, number, kNumberColor, alpha);
    }
}

}  // namespace wind::client
