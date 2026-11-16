// 이 파일은 무엇을 하나요?
// 칼자국을 만들고, 나이를 먹이고, 돌려서 그리는 곳이에요. 값의 이유는 docs/VISUALS.md "Slash arc" 절.

#include "game/presentation/slash_arc.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kFadeSeconds = 0.08f;     // 판정이 끝난 뒤 사라지는 시간
constexpr float kSweepDegrees = 50.0f;    // 휙 쓸 때 돌아가는 각도
constexpr float kSweepPart = 0.6f;        // 보이는 시간 중 쓸기에 쓰는 앞부분 (나머지는 제자리에서 흐려짐)
constexpr float kExtraLength = 6.0f;      // 히트박스보다 조금 길게 (칼끝이 상자에 딱 붙어 보이지 않게)
constexpr engine::Color kTint{220, 236, 255, 255};  // 살짝 푸른 흰색

// 바라보는 방향 -> 그림을 돌릴 각도 (그림은 오른쪽을 보고 있어요)
float facing_degrees(world::Direction facing) {
    switch (facing) {
        case world::Direction::Right: return 0.0f;
        case world::Direction::Down: return 90.0f;   // 화면은 아래가 +y라서 시계 방향 90도
        case world::Direction::Left: return 180.0f;
        case world::Direction::Up: return 270.0f;
    }
    return 0.0f;
}

bool horizontal(world::Direction facing) {
    return facing == world::Direction::Left || facing == world::Direction::Right;
}

}  // namespace

SlashArc::SlashArc(engine::TextureHandle texture) : texture_(texture) {}

void SlashArc::update(float dt_seconds, const AttackRunner& attack,
                      const std::optional<engine::RectF>& hitbox, world::Direction facing) {
    const bool active = attack.phase() == AttackRunner::Phase::Active && hitbox.has_value();
    if (active && !was_active_) {                          // 판정 구간이 막 시작됐어요
        box_ = *hitbox;                                    // 칼이 닿는 곳에
        facing_ = facing;                                  // 휘두른 방향으로
        reverse_ = attack.combo_index() % 2 == 1;          // 2타는 반대로 쓸어요
        lifetime_ = attack.current()->active + kFadeSeconds;  // 판정 동안 + 사라지는 시간
        age_ = 0.0f;
    }
    was_active_ = active;
    age_ += std::max(0.0f, dt_seconds);                    // 나이 먹기
}

void SlashArc::queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const {
    if (!visible() || !texture_.valid()) return;           // 안 보이거나 그림이 없으면 그만
    const float t = std::clamp(age_ / lifetime_, 0.0f, 1.0f);           // 0(처음) ~ 1(끝)
    const float sweep = std::min(1.0f, t / kSweepPart);                 // 쓸기 진행 0~1
    const float swing = (sweep - 0.5f) * kSweepDegrees * (reverse_ ? -1.0f : 1.0f);  // -25도 ~ +25도
    const float alpha = t < kSweepPart ? 1.0f : 1.0f - (t - kSweepPart) / (1.0f - kSweepPart);  // 끝에서 흐려짐

    // 그림은 "오른쪽을 볼 때" 모양이라, 돌리기 전 크기는 (앞뒤 길이, 옆 폭)이에요.
    const float length = (horizontal(facing_) ? box_.w : box_.h) + kExtraLength;
    const float width = horizontal(facing_) ? box_.h : box_.w;
    const engine::Vec2 center = camera.world_to_screen({box_.x + box_.w * 0.5f, box_.y + box_.h * 0.5f});

    engine::SpriteDraw draw;
    draw.texture = texture_;
    draw.dest = {std::round(center.x - length * 0.5f), std::round(center.y - width * 0.5f), length, width};
    draw.rotation_degrees = facing_degrees(facing_) + swing;   // 방향 + 쓸기
    draw.tint = kTint;
    draw.tint.a = static_cast<std::uint8_t>(std::lround(255.0f * std::clamp(alpha, 0.0f, 1.0f)));
    draw.blend = engine::BlendMode::Add;                       // 더하기 섞기: 빛나 보여요
    queue.push_sprite(engine::DrawLayer::Glow, 0.0f, draw);    // 조명 뒤라 밤·동굴에서도 밝아요
}

}  // namespace wind::game
