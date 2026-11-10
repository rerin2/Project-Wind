// 이 파일은 무엇을 하나요?
// 화면 가리기(페이드·아이리스)와 못 간 출구 알림을 그려요. (client_app.cpp에서 옮겨 왔어요)

#include "app/transition_view.hpp"

#include "engine/fx/iris_mask.hpp"

#include <algorithm>
#include <cmath>

namespace wind::client {
namespace {

// 아이리스 가리개 색 (불투명 검정)
constexpr engine::Color kTransitionColor{0, 0, 0, 255};
// 못 간 출구 알림: 글자 색(빨강), 화면 아래에서 띄울 거리, 띠 안쪽 여백, 최대 폭 (픽셀)
constexpr engine::Color kNoticeColor{255, 132, 120, 255};
constexpr engine::Color kNoticeBand{0, 0, 0, 200};
constexpr float kNoticeBottom = 30.0f;
constexpr float kNoticePadding = 4.0f;
constexpr float kNoticeMaxWidth = 560.0f;

}  // namespace

void draw_transition(engine::Renderer& renderer, const engine::ScreenFade& fade, engine::Vec2 focus) {
    const float darkness = fade.darkness();
    if (darkness <= 0.0f) {
        return;
    }
    const engine::Vec2 size = renderer.logical_size();
    if (fade.style() == world::ScreenTransition::Iris) {
        const float radius = engine::iris_radius(focus, size, darkness);
        for (const auto& rect : engine::iris_cover_rects(focus, radius, size)) {
            renderer.fill_rect(rect, kTransitionColor);
        }
        return;
    }
    renderer.fill_rect({0.0f, 0.0f, size.x, size.y},
                       {0, 0, 0, static_cast<std::uint8_t>(darkness * 255.0f)});
}

void draw_notice(engine::UiPainter& painter, std::string_view notice) {
    if (notice.empty()) {
        return;
    }
    const engine::Vec2 size = painter.renderer().logical_size();
    const float text_w = std::min(kNoticeMaxWidth, painter.measure(engine::FontStyle::Regular, notice));
    const float height = painter.line_height(engine::FontStyle::Regular);
    const float x = std::round((size.x - text_w) * 0.5f);
    const float y = size.y - kNoticeBottom;
    painter.renderer().fill_rect({x - kNoticePadding, y - kNoticePadding, text_w + kNoticePadding * 2.0f,
                                  height + kNoticePadding * 2.0f},
                                 kNoticeBand);
    engine::TextStyle style;
    style.color = kNoticeColor;
    style.max_lines = 1;
    painter.text({x, y, text_w, height}, notice, style);
}

}  // namespace wind::client
