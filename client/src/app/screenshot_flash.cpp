// 이 파일은 무엇을 하나요?
// "찰칵" 효과의 진하기를 트윈으로 줄여 가며 그리는 곳이에요.
// 숫자는 docs/VISUALS.md "Screenshot feedback" 표와 같아요.

#include "app/screenshot_flash.hpp"

#include <cstdint>
#include <string_view>

namespace wind::client {
namespace {

constexpr float kFlashSeconds = 0.30f;  // 번쩍임이 사라지는 시간
constexpr float kLabelSeconds = 1.20f;  // 글자가 사라지는 시간
constexpr float kFlashPeakAlpha = 170.0f;  // 번쩍임의 가장 진한 투명도 (255가 완전 하양)
constexpr float kMargin = 4.0f;
constexpr std::string_view kLabel = "screenshot saved";

std::uint8_t to_alpha(float strength, float peak) {
    return static_cast<std::uint8_t>(strength * peak);
}

}  // namespace

ScreenshotFlash::ScreenshotFlash()
    : flash_(1.0f, 0.0f, kFlashSeconds, engine::Ease::OutQuad),
      label_(1.0f, 0.0f, kLabelSeconds, engine::Ease::InCubic) {}

void ScreenshotFlash::trigger() {
    flash_.restart();
    label_.restart();
    active_ = true;
}

void ScreenshotFlash::update(float dt_seconds) {
    if (!active_) {
        return;
    }
    flash_.update(dt_seconds);
    label_.update(dt_seconds);
    active_ = !label_.finished();
}

void ScreenshotFlash::draw(engine::Renderer& renderer) const {
    if (!active_) {
        return;
    }
    const engine::Vec2 size = renderer.logical_size();
    if (!flash_.finished()) {
        renderer.fill_rect({0.0f, 0.0f, size.x, size.y},
                           {255, 255, 255, to_alpha(flash_.value(), kFlashPeakAlpha)});
    }
    // 오른쪽 아래 구석에 글자를 써요.
    const float width = static_cast<float>(kLabel.size()) * engine::Renderer::kDebugCharSize;
    const engine::Vec2 pos{size.x - width - kMargin,
                           size.y - engine::Renderer::kDebugCharSize - kMargin};
    renderer.draw_debug_text(pos, kLabel, {255, 255, 255, to_alpha(label_.value(), 255.0f)});
}

}  // namespace wind::client
