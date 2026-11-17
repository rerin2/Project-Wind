// 이 파일은 무엇을 하나요?
// 피격 비네트의 세기를 줄이고, 화면 전체를 덮는 붉은 가장자리를 그리기 목록에 넣는 곳이에요.

#include "game/presentation/hurt_vignette.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wind::game {
namespace {

constexpr float kFadeSeconds = 0.6f;                  // 맞은 뒤 사라지기까지 (무적 0.6초와 같아요)
constexpr float kPeakAlpha = 0.85f;                   // 가장 진할 때 (가장자리 기준)
constexpr engine::Color kTint{214, 28, 36, 255};      // 붉은색
constexpr float kSortKey = -1.0f;                     // 같은 단계(Foreground)에서 HP 바·느낌표보다 먼저(아래)

}  // namespace

void HurtVignette::update(float dt_seconds) {
    level_ = std::max(0.0f, level_ - std::max(0.0f, dt_seconds) / kFadeSeconds);
}

void HurtVignette::set_strength(float strength) {
    strength_ = std::isfinite(strength) ? std::clamp(strength, 0.0f, 1.0f) : 1.0f;
}

float HurtVignette::alpha() const {
    // 세기 제곱: 처음엔 확 붉다가 빨리 줄고, 끝은 천천히 사라져요
    return kPeakAlpha * level_ * level_ * strength_;
}

void HurtVignette::queue(engine::TextureHandle texture, engine::Vec2 viewport, engine::RenderQueue& queue) const {
    const float a = alpha();
    if (a <= 0.0f || !texture.valid()) return;  // 안 보이면 넣지 않아요
    engine::SpriteDraw draw;
    draw.texture = texture;
    draw.dest = {0.0f, 0.0f, viewport.x, viewport.y};  // 화면 전체
    draw.tint = kTint;
    draw.tint.a = static_cast<std::uint8_t>(std::lround(a * 255.0f));
    queue.push_sprite(engine::DrawLayer::Foreground, kSortKey, draw);
}

}  // namespace wind::game
