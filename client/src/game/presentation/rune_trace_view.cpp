// 이 파일은 무엇을 하나요?
// 룬 흔적 연출: 숨쉬는 금빛 웅덩이, 둘레를 도는 점, 피어오르는 가루, 금빛 빛, 되찾을 때 빨려 드는 알갱이.
// 수치의 이유는 docs/VISUALS.md "Rune trace" 절에 있어요.

#include "game/presentation/rune_trace_view.hpp"

#include "engine/fx/particle_spawn.hpp"

#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

constexpr float kPoolWidth = 26.0f;      // 웅덩이 가로 (땅에 납작한 타원)
constexpr float kPoolHeight = 10.0f;     // 웅덩이 세로
constexpr float kCoreSize = 9.0f;        // 가운데 밝은 점
constexpr float kBreathSpeed = 2.2f;     // 숨쉬기 빠르기 (라디안/초, 약 2.9초에 한 번)
constexpr float kOrbitRadiusX = 9.0f;    // 도는 점의 둘레 (가로)
constexpr float kOrbitRadiusY = 3.5f;    // 도는 점의 둘레 (세로: 땅에 누운 원이라 납작해요)
constexpr float kOrbitSpeed = 1.3f;      // 도는 빠르기 (라디안/초)
constexpr float kOrbitDotSize = 4.0f;    // 도는 점 크기
constexpr int kOrbitDots = 3;
constexpr float kEmitSeconds = 0.25f;    // 가루를 뿌리는 간격
constexpr float kLift = 3.0f;            // 가루가 나오는 높이 (발에서 위로)
constexpr float kLightRadius = 40.0f;    // 금빛 빛 크기
constexpr engine::Color kPoolColor{255, 196, 90, 150};   // 웅덩이 (금빛, 반투명)
constexpr engine::Color kCoreColor{255, 236, 170, 255};  // 가운데 (밝은 금빛)
constexpr engine::Color kLightColor{255, 206, 110, 255}; // 빛

// 둥근 빛 그림 하나를 더하기 섞기로 놓아요 (Glow 단계라 밤에도 밝아요)
void push_glow(const engine::Camera2D& camera, engine::TextureHandle glow, engine::Vec2 center, float w, float h,
               engine::Color tint, float sort_y, engine::RenderQueue& queue) {
    const engine::Vec2 screen = camera.world_to_screen(center);
    engine::SpriteDraw sprite;
    sprite.texture = glow;
    sprite.dest = {std::round(screen.x - w * 0.5f), std::round(screen.y - h * 0.5f), w, h};
    sprite.tint = tint;
    sprite.blend = engine::BlendMode::Add;
    queue.push_sprite(engine::DrawLayer::Glow, sort_y, sprite);
}

}  // namespace

void RuneTraceView::update(float dt_seconds, std::optional<engine::Vec2> trace_at, engine::Vec2 body,
                           engine::ParticleRandom& random, engine::ParticlePool& air_fx) {
    clock_ += dt_seconds;
    at_ = trace_at;
    orbs_.update(dt_seconds, body);  // 되찾은 알갱이는 흔적이 사라진 뒤에도 날아가요
    if (!at_ || motes_ == nullptr) return;
    emit_ -= dt_seconds;
    if (emit_ <= 0.0f) {  // 금빛 가루를 조금씩 (위로 피어올라요)
        emit_ = kEmitSeconds;
        engine::emit_burst(*motes_, *at_ - engine::Vec2{0.0f, kLift}, motes_->direction, random, air_fx);
    }
}

void RuneTraceView::recovered(engine::Vec2 at, std::int64_t amount, engine::ParticleRandom& random) {
    orbs_.spawn(at, amount, random);  // 13c 룬 알갱이: 튀어나왔다가 주인공에게 빨려 들어요
}

void RuneTraceView::queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const {
    orbs_.queue(camera, glow_, queue);
    if (!at_) return;
    const float breath = 0.5f + 0.5f * std::sin(clock_ * kBreathSpeed);  // 0~1 숨쉬기
    // 웅덩이: 땅에 누운 납작한 금빛, 숨쉬며 조금 커졌다 작아져요
    const float grow = 1.0f + 0.15f * breath;
    engine::Color pool = kPoolColor;
    pool.a = static_cast<std::uint8_t>(110 + 60 * breath);
    push_glow(camera, glow_, *at_, kPoolWidth * grow, kPoolHeight * grow, pool, at_->y, queue);
    push_glow(camera, glow_, *at_ - engine::Vec2{0.0f, 1.0f}, kCoreSize, kCoreSize * 0.6f, kCoreColor, at_->y, queue);
    // 도는 점 셋: 땅 위 납작한 원을 따라 천천히 돌아요
    for (int i = 0; i < kOrbitDots; ++i) {
        const float angle = clock_ * kOrbitSpeed + static_cast<float>(i) * 2.0f * std::numbers::pi_v<float> / kOrbitDots;
        const engine::Vec2 dot{at_->x + std::cos(angle) * kOrbitRadiusX, at_->y + std::sin(angle) * kOrbitRadiusY};
        push_glow(camera, glow_, dot, kOrbitDotSize, kOrbitDotSize, kCoreColor, dot.y, queue);
    }
}

std::vector<engine::Light> RuneTraceView::lights() const {
    if (!at_) return {};
    engine::Light light;
    light.position = *at_;
    light.radius = kLightRadius;
    light.color = kLightColor;
    light.intensity = 0.7f;
    light.pulse_frequency = kBreathSpeed / (2.0f * std::numbers::pi_v<float>);  // 웅덩이와 같은 박자로 숨쉬어요
    light.pulse_depth = 0.3f;
    return {light};
}

}  // namespace wind::game
