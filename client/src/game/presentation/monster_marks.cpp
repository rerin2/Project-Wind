// 이 파일은 무엇을 하나요?
// 느낌표와 공격 예고 바닥 표시를 그리기 목록에 넣는 곳이에요. 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

#include "game/presentation/monster_marks.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>

namespace wind::game {
namespace {

// 느낌표 모양: 3픽셀 폭 막대 7줄 + 1줄 틈 + 3x3 점, 둘레 1픽셀 짙은 테두리
constexpr float kMarkWidth = 3.0f;
constexpr float kBarHeight = 7.0f;
constexpr float kGap = 1.0f;
constexpr float kDotHeight = 3.0f;
constexpr float kOutline = 1.0f;
constexpr float kMarkGap = 3.0f;          // 머리 꼭대기와 느낌표 사이 (픽셀)
constexpr float kPopSeconds = 0.18f;      // 톡 튀어 오르는 시간
constexpr float kPopHeight = 5.0f;        // 튀어 오르는 높이 (픽셀)
constexpr engine::Color kMarkFill{255, 214, 74, 255};    // 노랑 (경고)
constexpr engine::Color kMarkOutline{16, 19, 28, 255};   // 짙은 남색 (HP 바 테두리와 같아요)

// 바닥 표시
constexpr float kStartScale = 0.55f;      // 예고 처음 크기 (끝 크기에 대한 비율)
constexpr float kStartAlpha = 70.0f;      // 예고 처음 진하기 (0~255)
constexpr float kEndAlpha = 190.0f;       // 예고 끝 진하기
constexpr float kWidthPad = 1.4f;         // 빛 그림 가장자리는 흐려서 조금 크게 그려요
constexpr float kCoreScale = 0.55f;       // 안쪽 진한 심의 크기
constexpr float kCoreAlpha = 0.6f;        // 안쪽 심의 진하기 (바깥 대비)
constexpr engine::Color kOuterColor{230, 52, 40, 255};   // 바깥: 붉은색
constexpr engine::Color kCoreColor{255, 128, 92, 255};   // 안쪽: 밝은 주황빛 빨강

// 0~1 투명도를 곱한 색
engine::Color with_alpha(engine::Color color, float alpha) {
    color.a = static_cast<std::uint8_t>(std::lround(std::clamp(alpha, 0.0f, 255.0f)));
    return color;
}

// 화면 위 네모 하나 (픽셀에 맞춰)
void fill(engine::RenderQueue& queue, float key, float x, float y, float w, float h, engine::Color color) {
    queue.push_fill(engine::DrawLayer::Foreground, key, {std::round(x), std::round(y), w, h}, color);
}

// 끝이 둥근 길쭉한 타원 하나 (빛 그림을 늘이고 돌려요)
void queue_ellipse(const engine::Camera2D& camera, engine::TextureHandle glow, engine::Vec2 center, float length,
                   float width, float degrees, engine::Color color, float sort_y, engine::RenderQueue& queue) {
    const engine::Vec2 screen = camera.world_to_screen(center);
    engine::SpriteDraw draw;
    draw.texture = glow;
    draw.dest = {std::round(screen.x - length * 0.5f), std::round(screen.y - width * 0.5f), length, width};
    draw.rotation_degrees = degrees;  // 가운데를 중심으로 덮칠 방향으로 돌려요
    draw.tint = color;
    queue.push_sprite(engine::DrawLayer::Shadows, sort_y, draw);  // 바닥 위, 몸 아래
}

// 예고 빛: 바닥 표시를 덮을 만큼, 진행할수록 밝게
constexpr float kLightRadiusScale = 0.9f;   // 빛 반지름 = (길이 + 폭) × 이 값
constexpr float kLightStart = 0.35f;        // 예고 처음 밝기
constexpr float kLightEnd = 0.9f;           // 예고 끝 밝기
constexpr engine::Color kLightColor{255, 84, 64, 255};  // 붉은 빛

}  // namespace

engine::Light telegraph_light(const TelegraphShape& shape, float progress) {
    const float p = std::clamp(progress, 0.0f, 1.0f);
    const engine::Vec2 path = shape.to - shape.from;
    engine::Light light;
    light.position = shape.from + path * 0.5f;                                      // 길 가운데
    light.radius = (engine::length(path) + shape.width) * kLightRadiusScale;        // 표시 전체를 비춰요
    light.color = kLightColor;
    light.intensity = kLightStart + (kLightEnd - kLightStart) * p;                  // 점점 밝게
    return light;
}

void queue_notice_mark(const engine::Camera2D& camera, engine::Vec2 head, float age, float alpha,
                       engine::RenderQueue& queue) {
    const float a = std::clamp(alpha, 0.0f, 1.0f) * 255.0f;
    if (a <= 0.0f) return;
    // 톡: 처음 0.18초 동안 위로 솟았다 내려앉아요
    const float pop = age < kPopSeconds ? std::sin(std::numbers::pi_v<float> * age / kPopSeconds) * kPopHeight : 0.0f;
    const engine::Vec2 screen = camera.world_to_screen(head);
    const float total = kBarHeight + kGap + kDotHeight;             // 느낌표 키
    const float left = screen.x - kMarkWidth * 0.5f;                // 가운데 맞춤
    const float top = screen.y - kMarkGap - total - pop;            // 머리 위
    const engine::Color outline = with_alpha(kMarkOutline, a);
    const engine::Color body = with_alpha(kMarkFill, a);
    // 테두리 한 장(막대와 점 사이 틈도 테두리 색) → 노란 속 (막대, 점). 같은 단계 안에서 키 순서대로 그려요.
    // 테두리를 두 장으로 그리면 틈 줄이 두 번 칠해져서, 흐려질 때 그 줄만 더 진해 보였어요 (11b 화면 확인).
    fill(queue, 0.0f, left - kOutline, top - kOutline, kMarkWidth + 2 * kOutline, total + 2 * kOutline, outline);
    fill(queue, 0.2f, left, top, kMarkWidth, kBarHeight, body);
    fill(queue, 0.3f, left, top + kBarHeight + kGap, kMarkWidth, kDotHeight, body);
}

void queue_telegraph(const engine::Camera2D& camera, engine::TextureHandle glow, const TelegraphShape& shape,
                     float progress, float sort_y, engine::RenderQueue& queue) {
    if (!glow.valid()) return;  // 그림이 없으면 표시만 빠져요 (예고 시간은 그대로라 피할 수는 있어요)
    const float p = std::clamp(progress, 0.0f, 1.0f);
    const float grow = 1.0f - (1.0f - p) * (1.0f - p);                      // 처음에 빨리 커지는 곡선
    const float scale = kStartScale + (1.0f - kStartScale) * grow;          // 크기 비율
    const float alpha = kStartAlpha + (kEndAlpha - kStartAlpha) * p;        // 점점 진하게
    const engine::Vec2 path = shape.to - shape.from;                        // 덮칠 길
    const engine::Vec2 center = shape.from + path * 0.5f;                   // 길 가운데
    const float length = (engine::length(path) + shape.width) * kWidthPad * scale;  // 끝이 둥글게 폭만큼 더
    const float width = shape.width * kWidthPad * scale;
    const float degrees = std::atan2(path.y, path.x) * 180.0f / std::numbers::pi_v<float>;
    queue_ellipse(camera, glow, center, length, width, degrees, with_alpha(kOuterColor, alpha), sort_y, queue);
    queue_ellipse(camera, glow, center, length * kCoreScale, width * kCoreScale, degrees,
                  with_alpha(kCoreColor, alpha * kCoreAlpha), sort_y, queue);
}

}  // namespace wind::game
