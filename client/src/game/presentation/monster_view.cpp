// 이 파일은 무엇을 하나요?
// 몬스터 그림자·그림·F4 상자를 그리기 목록에 넣는 곳이에요. 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

#include "game/presentation/monster_view.hpp"

#include "game/presentation/monster_pose.hpp"
#include "game/presentation/shadow_drawing.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace wind::game {
namespace {

// 발밑 그림자 (슬라임은 납작하고 넓어서 몸통 폭 정도)
constexpr float kShadowWidth = 18.0f;
constexpr float kShadowHeight = 6.0f;
constexpr float kShadowAirShrink = 0.08f;  // 1픽셀 뜰 때마다 그림자가 8%씩 작아져요
// 숨쉬기를 마리마다 엇갈리게: 몇 번째인지 × 이 값 (황금비 조각이라 겹치지 않고 고르게 흩어져요)
constexpr float kPhaseStep = 0.618f;
// F4 상자 색 (반투명)
constexpr engine::Color kHurtboxColor{240, 210, 60, 100};  // 맞는 상자: 노랑 (허수아비와 같아요)
constexpr engine::Color kBodyColor{80, 200, 255, 140};     // 발 상자: 하늘색 (주인공과 같아요)

// 맵 위 네모 -> 화면 위 네모
engine::RectF to_screen(const engine::Camera2D& camera, const engine::RectF& world) {
    const engine::Vec2 top_left = camera.world_to_screen({world.x, world.y});
    return {top_left.x, top_left.y, world.w, world.h};
}

}  // namespace

MonsterView::MonsterView(engine::TextureHandle shadow, std::vector<engine::TextureHandle> textures)
    : shadow_(shadow), textures_(std::move(textures)), hop_blend_(textures_.size(), 0.0f) {}

void MonsterView::update(float dt_seconds, std::span<const Monster> monsters) {
    clock_ += std::max(0.0f, dt_seconds);  // 숨쉬기 시계
    const std::size_t count = std::min(monsters.size(), hop_blend_.size());
    for (std::size_t i = 0; i < count; ++i) {
        const bool walking = monsters[i].state() == MonsterState::Wander;  // 걸으면 뛰어요
        hop_blend_[i] = approach_hop_blend(hop_blend_[i], walking, dt_seconds);
    }
}

void MonsterView::queue(const engine::Camera2D& camera, std::span<const Monster> monsters, float alpha,
                        bool show_boxes, engine::RenderQueue& queue) const {
    const std::size_t count = std::min(monsters.size(), textures_.size());  // 개수가 다르면 적은 쪽까지만
    for (std::size_t i = 0; i < count; ++i) {
        queue_monster(camera, monsters[i], i, alpha, queue);
        if (show_boxes) queue_boxes(camera, monsters[i], queue);
    }
}

void MonsterView::queue_monster(const engine::Camera2D& camera, const Monster& monster, std::size_t index,
                                float alpha, engine::RenderQueue& queue) const {
    const engine::Vec2 feet = monster.interpolated_feet(alpha);       // 두 박자 사이 위치
    const float phase = std::fmod(static_cast<float>(index) * kPhaseStep, 1.0f);
    const MonsterPose pose = slime_pose(clock_, phase, monster.distance_moved(), hop_blend_[index]);
    // 그림자: 땅에 그대로 두고, 몸이 뜨면 작아져요
    const float shrink = std::max(0.5f, 1.0f - pose.lift * kShadowAirShrink);
    queue_shadow(camera, shadow_, feet, {kShadowWidth * shrink, kShadowHeight * shrink}, queue);
    // 몸: 그림 속 발 위치(origin)를 기준으로 늘이고 줄여서, 발은 늘 땅(또는 뜬 높이)에 붙어 있어요
    const auto& sprite = monster.data().sprite;
    const engine::Vec2 screen = camera.world_to_screen(feet);
    const float width = std::round(static_cast<float>(sprite.frame_width) * pose.scale_x);   // 픽셀 단위로 맞춰
    const float height = std::round(static_cast<float>(sprite.frame_height) * pose.scale_y);  // 또렷하게
    const float origin_x = static_cast<float>(sprite.origin_x) * width / static_cast<float>(sprite.frame_width);
    const float origin_y = static_cast<float>(sprite.origin_y) * height / static_cast<float>(sprite.frame_height);
    engine::SpriteDraw draw;
    draw.texture = textures_[index];
    draw.source = {0, 0, sprite.frame_width, sprite.frame_height};  // 0번 칸 = 보통 모습 (1번 웅크림은 11b)
    draw.dest = {std::round(screen.x - origin_x), std::round(screen.y - pose.lift - origin_y), width, height};
    queue.push_sprite(engine::DrawLayer::YSorted, feet.y, draw);  // 주인공·허수아비와 발 위치로 줄 서기
}

void MonsterView::queue_boxes(const engine::Camera2D& camera, const Monster& monster,
                              engine::RenderQueue& queue) const {
    // 상자는 판정과 같은 "지금 박자" 위치에 그려요 (보간하지 않아요: 판정이 실제로 쓰는 자리)
    queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, to_screen(camera, monster.hurtbox()), kHurtboxColor);
    queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, to_screen(camera, body_box(monster.feet(), monster.body())),
                    kBodyColor);
}

}  // namespace wind::game
