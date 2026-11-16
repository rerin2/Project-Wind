// 이 파일은 무엇을 하나요?
// 피해 숫자를 숫자마다 작은 글 상자에 가운데 맞춰 그리는 곳이에요.
// 색 값의 이유는 docs/VISUALS.md "Hit feel" 절에 있어요.

#include "app/damage_number_drawing.hpp"

#include <cmath>
#include <string>

namespace wind::client {
namespace {

constexpr float kBoxWidth = 48.0f;                     // 숫자 상자 너비 (다섯 자리도 들어가요)
constexpr engine::Color kCritColor{242, 181, 68, 255};  // 팔레트 gold #f2b544

}  // namespace

void draw_damage_numbers(engine::UiPainter& painter, const engine::Camera2D& camera,
                         const game::DamageNumbers& numbers) {
    for (const auto& number : numbers.numbers()) {
        const engine::Vec2 screen = camera.world_to_screen(numbers.position(number));  // 맵 위 -> 화면
        engine::TextStyle style;
        style.font = number.critical ? engine::FontStyle::Bold : engine::FontStyle::Regular;
        style.color = number.critical ? kCritColor : painter.theme().text;  // 치명타는 금색
        style.align = engine::TextAlign::Center;
        style.max_lines = 1;
        style.wrap = false;
        const float height = painter.line_height(style.font);
        // 숫자의 가운데 아래가 위치에 오도록 상자를 놓아요 (정수 픽셀로 맞춰 흔들리지 않게)
        const engine::RectF box{std::round(screen.x - kBoxWidth * 0.5f), std::round(screen.y - height),
                                kBoxWidth, height};
        const std::string text = std::to_string(number.amount) + (number.critical ? "!" : "");
        painter.text(box, text, style, numbers.alpha(number));
    }
}

}  // namespace wind::client
