// 이 파일은 무엇을 하나요?
// 저장 아이콘 그리기: 점 그림 잎 + 그림자 + 둘레를 도는 반짝이.

#include "app/save_icon_view.hpp"

#include "engine/render/renderer.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/ui/save_icon.hpp"

#include <array>
#include <cmath>
#include <numbers>
#include <string_view>

namespace wind::client {
namespace {

// 캣닢 잎 점 그림 (9×11). '#' = 잎, '+' = 밝은 잎맥, '.' = 빈칸. 위가 잎 끝, 아래가 줄기.
constexpr std::array<std::string_view, 11> kLeaf{
    "....#....",
    "...###...",
    "..##+##..",
    ".###+###.",
    "####+####",
    "####+####",
    ".###+###.",
    "..##+##..",
    "...#+#...",
    "....+....",
    "....+....",
};
constexpr float kPixel = 2.0f;                       // 점 하나 = 화면 2픽셀
constexpr float kMargin = 10.0f;                     // 화면 오른쪽·위에서 띄우는 거리
constexpr engine::Color kLeafColor{232, 190, 92, 255};   // 금빛 잎 (룬 숫자와 같은 계열)
constexpr engine::Color kVeinColor{255, 240, 190, 255};  // 밝은 잎맥
constexpr engine::Color kShadow{20, 14, 30, 150};        // 아래 그림자 (밝은 낮에도 보이게)
constexpr float kShadowDrop = 1.0f;                  // 그림자를 내리는 거리 (화면 픽셀)
constexpr engine::Color kSparkColor{255, 250, 220, 255}; // 도는 반짝이
constexpr float kBobPixels = 1.0f;                   // 잎이 위아래로 흔들리는 폭 (점 단위가 아니라 화면 픽셀)
constexpr float kBobHz = 1.5f;                       // 흔들림 빠르기 (초당)
constexpr float kOrbitTurnsPerSecond = 0.9f;         // 반짝이가 도는 빠르기 (초당 바퀴)
constexpr float kOrbitRadius = 13.0f;                // 반짝이가 도는 반지름 (화면 픽셀)
constexpr float kSparkSize = 2.0f;                   // 반짝이 크기

// 점 그림을 at(왼쪽 위)부터 그려요. shadow면 모든 점을 그림자 색으로
void draw_leaf(engine::Renderer& renderer, engine::Vec2 at, float alpha, bool shadow) {
    for (std::size_t row = 0; row < kLeaf.size(); ++row) {
        for (std::size_t col = 0; col < kLeaf[row].size(); ++col) {
            const char dot = kLeaf[row][col];
            if (dot == '.') continue;  // 빈칸
            const engine::Color color = shadow ? kShadow : (dot == '+' ? kVeinColor : kLeafColor);
            renderer.fill_rect({at.x + static_cast<float>(col) * kPixel, at.y + static_cast<float>(row) * kPixel,
                                kPixel, kPixel},
                               engine::with_alpha(color, alpha));
        }
    }
}

}  // namespace

void draw_save_icon(engine::Renderer& renderer, engine::Vec2 screen, const game::SaveIcon& icon) {
    if (!icon.visible()) return;
    const float alpha = icon.alpha();
    const float t = icon.shown_seconds();
    const float width = static_cast<float>(kLeaf[0].size()) * kPixel;
    const float height = static_cast<float>(kLeaf.size()) * kPixel;
    // 위아래 흔들림: 정수 픽셀로 반올림해서 점 그림이 흐릿해지지 않게
    const float bob = std::round(std::sin(t * kBobHz * 2.0f * std::numbers::pi_v<float>) * kBobPixels);
    const engine::Vec2 leaf{std::round(screen.x - kMargin - width), std::round(kMargin + kOrbitRadius * 0.5f) + bob};
    draw_leaf(renderer, {leaf.x, leaf.y + kShadowDrop}, alpha, true);  // 그림자 (조금 아래)
    draw_leaf(renderer, leaf, alpha, false);
    // 반짝이: 잎 가운데를 중심으로 시계 방향으로 돌아요
    const float angle = t * kOrbitTurnsPerSecond * 2.0f * std::numbers::pi_v<float>;
    const engine::Vec2 center{leaf.x + width * 0.5f, leaf.y + height * 0.5f};
    const engine::Vec2 spark{std::round(center.x + std::cos(angle) * kOrbitRadius - kSparkSize * 0.5f),
                             std::round(center.y + std::sin(angle) * kOrbitRadius - kSparkSize * 0.5f)};
    renderer.fill_rect({spark.x, spark.y, kSparkSize, kSparkSize}, engine::with_alpha(kSparkColor, alpha));
}

}  // namespace wind::client
