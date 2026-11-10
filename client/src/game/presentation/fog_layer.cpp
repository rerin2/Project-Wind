// 이 파일은 무엇을 하나요?
// 안개 덩어리들의 위치를 시간에 따라 옮기고, 그리기 목록에 넣는 곳이에요.
// 덩어리는 화면 기준으로 흘러가요. 오른쪽 끝을 넘어가면 왼쪽 끝에서 다시 나와요.

#include "game/presentation/fog_layer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace wind::game {
namespace {

// 안개 덩어리 하나: 처음 위치(화면 너비·높이에 대한 비율), 크기(픽셀), 흐르는 빠르기(픽셀/초)
struct FogBank {
    float x;
    float y;
    float width;
    float height;
    float speed;
};

constexpr std::array<FogBank, 7> kBanks{{
    {0.05f, 0.10f, 300.0f, 120.0f, 6.0f},
    {0.45f, 0.25f, 360.0f, 140.0f, 4.0f},
    {0.80f, 0.05f, 280.0f, 110.0f, 7.0f},
    {0.20f, 0.55f, 340.0f, 130.0f, 5.0f},
    {0.65f, 0.60f, 300.0f, 120.0f, 8.0f},
    {0.95f, 0.45f, 260.0f, 100.0f, 5.5f},
    {0.35f, 0.85f, 380.0f, 140.0f, 4.5f},
}};
constexpr engine::Color kFogColor{214, 220, 232, 255};
constexpr float kMaxAlpha = 120.0f;  // 짙기 1일 때 덩어리 하나의 가장 진한 투명도
constexpr float kMargin = 200.0f;    // 화면 밖에서 들어오고 나가는 여유 (픽셀)

}  // namespace

void FogLayer::queue(const engine::Camera2D& camera, float fog, engine::TextureHandle texture,
                     engine::RenderQueue& queue) const {
    if (fog <= 0.0f || !texture.valid()) {
        return;
    }
    const engine::Vec2 size = camera.viewport();
    const float span = size.x + kMargin * 2.0f;
    const engine::RectF view = camera.view_rect();
    const auto alpha = static_cast<std::uint8_t>(std::lround(std::min(fog, 1.0f) * kMaxAlpha));
    for (const FogBank& bank : kBanks) {
        // 옆으로 흘러가다가 오른쪽 끝을 넘으면 왼쪽으로 돌아와요.
        const float x = std::fmod(bank.x * span + bank.speed * time_, span) - kMargin;
        const float y = bank.y * size.y - bank.height * 0.5f;
        engine::SpriteDraw sprite;
        sprite.texture = texture;
        sprite.dest = {std::round(x - bank.width * 0.5f), std::round(y), bank.width, bank.height};
        sprite.tint = {kFogColor.r, kFogColor.g, kFogColor.b, alpha};
        // 공중 효과 중에서도 맨 나중(가장 위)에 그려지게 화면 아래쪽 끝 값을 줘요.
        queue.push_sprite(engine::DrawLayer::WorldFx, view.bottom(), sprite);
    }
}

}  // namespace wind::game
