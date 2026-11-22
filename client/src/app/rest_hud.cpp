// 이 파일은 무엇을 하나요?
// "[E] 쉬기" 안내와 "주변이 너무 소란스럽다" 알림을 갱신하고 그려요.
// 색과 자리 값의 이유는 docs/VISUALS.md "Cat-nip cushion" 절에 있어요.

#include "app/rest_hud.hpp"

#include "engine/ui/ui_painter.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace wind::client {
namespace {

constexpr std::string_view kPromptText = "[E] 쉬기";
constexpr std::string_view kNoticeText = "주변이 너무 소란스럽다";
constexpr float kPromptFadePerSecond = 6.0f;   // 안내가 나타나고 사라지는 빠르기 (약 0.17초)
constexpr float kPromptBottom = 64.0f;         // 화면 아래에서 안내 글자 위쪽까지 (주인공 발 아래쯤)
constexpr float kPromptRise = 4.0f;            // 나타날 때 아래에서 떠오르는 거리 (픽셀)
constexpr float kNoticeSeconds = 1.8f;         // 알림을 보여 주는 시간
constexpr float kNoticeFadeSeconds = 0.3f;     // 알림이 사라질 때 흐려지는 시간
constexpr float kNoticeTop = 0.62f;            // 알림 자리 (화면 높이의 비율, 주인공 아래)
constexpr engine::Color kPromptColor{240, 232, 200, 255};  // 크림색 (밝은 글자)
constexpr engine::Color kNoticeColor{236, 156, 96, 255};   // 주황 (경고지만 빨강보다 부드럽게)
constexpr engine::Color kBand{0, 0, 0, 110};               // 글자 뒤 반투명 띠
constexpr float kBandPadX = 8.0f;                           // 띠 양옆 여백
constexpr float kBandPadY = 2.0f;                           // 띠 위아래 여백

// 화면 가운데에 띠를 깔고 글자를 한 줄 그려요
void draw_line(engine::UiPainter& painter, std::string_view text, float y, engine::Color color, float alpha,
               engine::Vec2 screen) {
    const auto font = engine::FontStyle::Regular;
    const float w = painter.measure(font, text);
    const float h = painter.line_height(font);
    const float x = std::round((screen.x - w) * 0.5f);  // 정수 픽셀 (흐릿하지 않게)
    painter.renderer().fill_rect({x - kBandPadX, y - kBandPadY, w + kBandPadX * 2.0f, h + kBandPadY * 2.0f},
                                 engine::with_alpha(kBand, alpha));
    engine::TextStyle style;
    style.font = font;
    style.color = color;
    style.align = engine::TextAlign::Center;
    style.max_lines = 1;
    painter.text({x, y, w, h}, text, style, alpha);
}

}  // namespace

void RestHud::update(float dt_seconds, bool prompt, unsigned denied) {
    // 안내: 보여 줄 때는 1까지, 아니면 0까지 일정한 빠르기로 (갑자기 튀지 않게)
    const float target = prompt ? 1.0f : 0.0f;
    const float step = kPromptFadePerSecond * dt_seconds;
    prompt_alpha_ = prompt_alpha_ < target ? std::min(target, prompt_alpha_ + step)
                                           : std::max(target, prompt_alpha_ - step);
    // 알림: 횟수가 늘었으면 다시 보여 줘요. 처음 본 값과, 새 맵에서 0으로 돌아간 값은 알리지 않아요.
    if (denied_seen_ && denied > *denied_seen_) notice_seconds_ = kNoticeSeconds;
    denied_seen_ = denied;
    notice_seconds_ = std::max(0.0f, notice_seconds_ - dt_seconds);
}

void RestHud::draw(engine::UiPainter& painter, engine::Vec2 screen) const {
    if (prompt_alpha_ > 0.0f) {
        const float y = std::round(screen.y - kPromptBottom + kPromptRise * (1.0f - prompt_alpha_));  // 떠오르기
        draw_line(painter, kPromptText, y, kPromptColor, prompt_alpha_, screen);
    }
    if (notice_seconds_ > 0.0f) {
        const float alpha = std::min(1.0f, notice_seconds_ / kNoticeFadeSeconds);  // 끝에서 흐려져요
        draw_line(painter, kNoticeText, std::round(screen.y * kNoticeTop), kNoticeColor, alpha, screen);
    }
}

}  // namespace wind::client
