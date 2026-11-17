// 이 파일은 무엇을 하나요?
// 적 머리 위 HP 바(테두리·빈 곳·꼬리·채움)를 그리기 목록에 넣는 곳이에요. (combat_view.cpp에서 옮겨 왔어요)

#include "game/presentation/enemy_bar_drawing.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wind::game {
namespace {

// HP 바: 머리 위 3픽셀, 테두리 포함 26x5 (안쪽 24x3)
constexpr float kBarWidth = 26.0f;
constexpr float kBarHeight = 5.0f;
constexpr float kBarGap = 3.0f;
constexpr engine::Color kBarOutline{16, 19, 28, 255};
constexpr engine::Color kBarEmpty{40, 44, 60, 255};
constexpr engine::Color kBarFill{214, 72, 64, 255};    // 빨강 (적 HP)
constexpr engine::Color kBarTrail{250, 238, 196, 255};  // 방금 깎인 만큼 (크림색)

// 투명도(0~1)를 곱한 색
engine::Color faded(engine::Color color, float alpha) {
    color.a = static_cast<std::uint8_t>(std::lround(static_cast<float>(color.a) * std::clamp(alpha, 0.0f, 1.0f)));
    return color;
}

}  // namespace

DelayedBarTuning enemy_bar_tuning() {
    DelayedBarTuning tuning;
    tuning.auto_hide = true;           // 가득 차면 숨기기
    tuning.hide_after_seconds = 2.5f;  // 마지막으로 맞고 다시 찬 뒤 2.5초
    tuning.hidden_alpha = 0.0f;        // 완전히 안 보이게
    tuning.start_hidden = true;        // 맵에 들어왔을 때 가득 찬 바는 처음부터 안 보여요
    return tuning;
}

void queue_enemy_bar(const engine::Camera2D& camera, engine::Vec2 feet, float head_height, const DelayedBar& bar,
                     engine::RenderQueue& queue) {
    const float alpha = bar.alpha();
    if (alpha <= 0.0f) return;                                  // 숨겨진 바는 그리지 않아요
    const engine::Vec2 screen = camera.world_to_screen(feet);
    const engine::RectF outer{std::round(screen.x - kBarWidth * 0.5f),        // 머리 위 가운데
                              std::round(screen.y - head_height - kBarGap - kBarHeight), kBarWidth, kBarHeight};
    const engine::RectF inner{outer.x + 1.0f, outer.y + 1.0f, outer.w - 2.0f, outer.h - 2.0f};
    // 조명 뒤(Foreground)에 그려서 밤·동굴에서도 잘 보여요. 그리는 순서는 같은 단계 안에서 키 순서예요.
    queue.push_fill(engine::DrawLayer::Foreground, 0.0f, outer, faded(kBarOutline, alpha));
    queue.push_fill(engine::DrawLayer::Foreground, 0.1f, inner, faded(kBarEmpty, alpha));
    queue.push_fill(engine::DrawLayer::Foreground, 0.2f,
                    {inner.x, inner.y, std::round(inner.w * bar.trail()), inner.h}, faded(kBarTrail, alpha));
    queue.push_fill(engine::DrawLayer::Foreground, 0.3f,
                    {inner.x, inner.y, std::round(inner.w * bar.value()), inner.h}, faded(kBarFill, alpha));
}

}  // namespace wind::game
