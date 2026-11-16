// 이 파일은 무엇을 하나요?
// 허수아비 그림, 그림자, 머리 위 HP 바, F4 판정 상자를 그리기 목록에 넣는 곳이에요.
// 색과 크기 값의 이유는 docs/VISUALS.md "Training dummy" 절에 있어요.

#include "game/presentation/combat_view.hpp"

#include "game/presentation/shadow_drawing.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace wind::game {
namespace {

// 발밑 그림자 크기 (주인공 22x8보다 살짝 작게)
constexpr float kShadowWidth = 20.0f;
constexpr float kShadowHeight = 7.0f;
// HP 바: 머리 위 3픽셀, 테두리 포함 26x5 (안쪽 24x3)
constexpr float kBarWidth = 26.0f;
constexpr float kBarHeight = 5.0f;
constexpr float kBarGap = 3.0f;
constexpr engine::Color kBarOutline{16, 19, 28, 255};
constexpr engine::Color kBarEmpty{40, 44, 60, 255};
constexpr engine::Color kBarFill{214, 72, 64, 255};    // 빨강 (적 HP)
constexpr engine::Color kBarTrail{250, 238, 196, 255};  // 방금 깎인 만큼 (크림색)
// 맞을 때 하얀 번쩍임 (명세 6.7 기본값 0.08초)
constexpr float kFlashSeconds = 0.08f;
// F4 판정 상자 색 (반투명)
constexpr engine::Color kHitboxColor{240, 60, 60, 110};    // 칼: 빨강
constexpr engine::Color kHurtboxColor{240, 210, 60, 100};  // 허수아비: 노랑

// 허수아비 HP 바: 가득 찬 채 2.5초면 완전히 사라져요 (서 있는 허수아비마다 바가 떠 있으면 어지러워서)
DelayedBarTuning bar_tuning() {
    DelayedBarTuning tuning;
    tuning.auto_hide = true;           // 가득 차면 숨기기
    tuning.hide_after_seconds = 2.5f;  // 마지막으로 맞고 다시 찬 뒤 2.5초
    tuning.hidden_alpha = 0.0f;        // 완전히 안 보이게
    tuning.start_hidden = true;        // 맵에 들어왔을 때 가득 찬 바는 처음부터 안 보여요
    return tuning;
}

// 투명도(0~1)를 곱한 색
engine::Color faded(engine::Color color, float alpha) {
    color.a = static_cast<std::uint8_t>(std::lround(static_cast<float>(color.a) * std::clamp(alpha, 0.0f, 1.0f)));
    return color;
}

// 맵 위 네모 -> 화면 위 네모
engine::RectF to_screen(const engine::Camera2D& camera, const engine::RectF& world) {
    const engine::Vec2 top_left = camera.world_to_screen({world.x, world.y});
    return {top_left.x, top_left.y, world.w, world.h};
}

}  // namespace

CombatView::CombatView(engine::TextureHandle shadow, std::vector<engine::TextureHandle> dummy_textures)
    : shadow_(shadow), textures_(std::move(dummy_textures)) {
    for (std::size_t i = 0; i < textures_.size(); ++i) {
        bars_.emplace_back(bar_tuning(), 1.0f);  // 허수아비마다 바 하나 (가득 찬 채 시작)
    }
    flash_left_.assign(textures_.size(), 0.0f);  // 처음에는 아무도 안 번쩍여요
}

void CombatView::flash(std::span<const TrainingDummy> dummies, TargetId target) {
    const std::size_t count = std::min(dummies.size(), flash_left_.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (dummies[i].id() == target) {
            flash_left_[i] = kFlashSeconds;  // 맞은 허수아비를 처음부터 다시 번쩍
        }
    }
}

void CombatView::set_flash_strength(float strength) {
    flash_strength_ = std::isfinite(strength) ? std::clamp(strength, 0.0f, 1.0f) : 1.0f;
}

float CombatView::flash_amount(std::size_t index) const {
    if (index >= flash_left_.size()) return 0.0f;
    return flash_strength_ * std::clamp(flash_left_[index] / kFlashSeconds, 0.0f, 1.0f);  // 끝으로 갈수록 흐려져요
}

void CombatView::update(float dt_seconds, std::span<const TrainingDummy> dummies) {
    const std::size_t count = std::min(dummies.size(), bars_.size());  // 개수가 다르면 적은 쪽까지만
    for (std::size_t i = 0; i < count; ++i) {
        const auto& dummy = dummies[i];
        const float fraction = dummy.max_hp() > 0
                                   ? static_cast<float>(dummy.hp()) / static_cast<float>(dummy.max_hp())
                                   : 0.0f;          // 0~1
        bars_[i].set(fraction);                     // 줄면 바로 줄고 꼬리를 남겨요
        bars_[i].update(dt_seconds);                // 꼬리 따라가기, 차오르기, 숨기기
    }
    for (auto& left : flash_left_) {                // 번쩍임 시간 줄이기
        left = std::max(0.0f, left - std::max(0.0f, dt_seconds));
    }
}

void CombatView::queue(const engine::Camera2D& camera, std::span<const TrainingDummy> dummies,
                       const std::optional<engine::RectF>& hitbox, bool show_boxes,
                       engine::RenderQueue& queue) const {
    const std::size_t count = std::min(dummies.size(), bars_.size());
    for (std::size_t i = 0; i < count; ++i) {
        queue_dummy(camera, dummies[i], i, queue);         // 그림자 + 허수아비
        queue_bar(camera, dummies[i], bars_[i], queue);    // 머리 위 HP 바
    }
    if (!show_boxes) return;                               // F4가 꺼져 있으면 여기까지
    for (const auto& dummy : dummies) {
        if (dummy.can_be_hit()) {                          // 맞을 수 있는 허수아비만 노랗게
            queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, to_screen(camera, dummy.hurtbox()),
                            kHurtboxColor);
        }
    }
    if (hitbox) {                                          // 칼이 판정 중일 때만 빨갛게
        queue.push_fill(engine::DrawLayer::DebugOverlay, 1.0f, to_screen(camera, *hitbox), kHitboxColor);
    }
}

void CombatView::queue_dummy(const engine::Camera2D& camera, const TrainingDummy& dummy,
                             std::size_t index, engine::RenderQueue& queue) const {
    const auto& sprite = dummy.data().sprite;                   // 그림 설명 (칸 크기, 발 위치)
    // 그림은 밀린 몸통 위치에 (넉백 흔들림, 10a). 그림자는 박힌 자리 그대로라 "밀렸다"가 잘 보여요.
    const engine::Vec2 screen = camera.world_to_screen(dummy.feet() + dummy.body_offset());
    queue_shadow(camera, shadow_, dummy.feet(), {kShadowWidth, kShadowHeight}, queue);  // 발밑 그림자
    engine::SpriteDraw draw;
    draw.texture = textures_[index];                            // 이 허수아비 그림
    const int frame = dummy.down() ? 1 : 0;                     // 0 = 서 있음, 1 = 쓰러짐
    draw.source = {frame * sprite.frame_width, 0, sprite.frame_width, sprite.frame_height};
    draw.dest = {std::round(screen.x - static_cast<float>(sprite.origin_x)),   // 그림 속 발이 진짜 발에 오게
                 std::round(screen.y - static_cast<float>(sprite.origin_y)),
                 static_cast<float>(sprite.frame_width), static_cast<float>(sprite.frame_height)};
    queue.push_sprite(engine::DrawLayer::YSorted, dummy.feet().y, draw);  // 주인공과 발 위치로 줄 서기
    const float flash = flash_amount(index);
    if (flash > 0.0f) {  // 번쩍임: 같은 그림을 하얗게 더해요. 줄 서기 값이 같으면 나중에 넣은 게 위에 그려져요
        draw.blend = engine::BlendMode::Add;
        draw.tint = {255, 255, 255, static_cast<std::uint8_t>(std::lround(255.0f * flash))};
        queue.push_sprite(engine::DrawLayer::YSorted, dummy.feet().y, draw);
    }
}

void CombatView::queue_bar(const engine::Camera2D& camera, const TrainingDummy& dummy,
                           const DelayedBar& bar, engine::RenderQueue& queue) const {
    const float alpha = bar.alpha();
    if (alpha <= 0.0f) return;                                  // 숨겨진 바는 그리지 않아요
    const auto& sprite = dummy.data().sprite;
    const engine::Vec2 screen = camera.world_to_screen(dummy.feet());
    const engine::RectF outer{std::round(screen.x - kBarWidth * 0.5f),        // 머리 위 가운데
                              std::round(screen.y - static_cast<float>(sprite.origin_y) - kBarGap - kBarHeight),
                              kBarWidth, kBarHeight};
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
