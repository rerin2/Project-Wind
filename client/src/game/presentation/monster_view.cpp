// 이 파일은 무엇을 하나요?
// 몬스터 그림자·몸·느낌표·공격 예고 바닥 표시·F4 상자를 그리기 목록에 넣는 곳이에요.
// 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

#include "game/presentation/monster_view.hpp"

#include "game/presentation/enemy_bar_drawing.hpp"
#include "game/presentation/monster_marks.hpp"
#include "game/presentation/monster_pose.hpp"
#include "game/presentation/shadow_drawing.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

namespace wind::game {
namespace {

// 발밑 그림자 (슬라임은 납작하고 넓어서 몸통 폭 정도)
constexpr float kShadowWidth = 18.0f;
constexpr float kShadowHeight = 6.0f;
constexpr float kShadowAirShrink = 0.08f;  // 1픽셀 뜰 때마다 그림자가 8%씩 작아져요
// 숨쉬기를 마리마다 엇갈리게: 몇 번째인지 × 이 값 (황금비 조각이라 겹치지 않고 고르게 흩어져요)
constexpr float kPhaseStep = 0.618f;
// 느낌표는 알아챈 뒤 쫓기 시작하고도 이만큼 더 보이며 흐려져요 (초). 알아채기 0.4초만으로는 짧아서요.
constexpr float kMarkLingerSeconds = 0.35f;
// 맞을 때 하얀 번쩍임 (명세 6.7 기본값 0.08초, 허수아비와 같아요, 11d)
constexpr float kFlashSeconds = 0.08f;
// 쓰러지는 동안 깜빡임: 0.1초마다 한 번, 밝기 70% (명세 9.5 사라짐 연출, 11d)
constexpr float kDeathBlinkPeriod = 0.1f;
constexpr float kDeathBlinkAlpha = 0.7f;
// F4 상자 색 (반투명)
constexpr engine::Color kHurtboxColor{240, 210, 60, 100};  // 맞는 상자: 노랑 (허수아비와 같아요)
constexpr engine::Color kBodyColor{80, 200, 255, 140};     // 발 상자: 하늘색 (주인공과 같아요)
constexpr engine::Color kHitboxColor{240, 60, 60, 110};    // 공격 판정: 빨강 (칼과 같아요)

// 맵 위 네모 -> 화면 위 네모
engine::RectF to_screen(const engine::Camera2D& camera, const engine::RectF& world) {
    const engine::Vec2 top_left = camera.world_to_screen({world.x, world.y});
    return {top_left.x, top_left.y, world.w, world.h};
}

// 느낌표 진하기 0~1: 알아채는 동안 1, 쫓기 시작하면 잠깐 뒤 흐려져요. 그 밖에는 0
float notice_alpha(const Monster& monster) {
    if (monster.state() == MonsterState::Notice) return 1.0f;
    if (monster.state() == MonsterState::Chase && monster.state_seconds() < kMarkLingerSeconds) {
        return 1.0f - monster.state_seconds() / kMarkLingerSeconds;
    }
    return 0.0f;
}

}  // namespace

MonsterView::MonsterView(engine::TextureHandle shadow, engine::TextureHandle glow,
                         std::vector<engine::TextureHandle> textures)
    : shadow_(shadow), glow_(glow), textures_(std::move(textures)), hop_blend_(textures_.size(), 0.0f),
      notice_age_(textures_.size(), 0.0f), flash_left_(textures_.size(), 0.0f) {
    for (std::size_t i = 0; i < textures_.size(); ++i) {
        bars_.emplace_back(enemy_bar_tuning(), 1.0f);  // 몬스터마다 HP 바 하나 (가득 찬 채 숨어 있어요, 11d)
    }
}

void MonsterView::flash(std::span<const Monster> monsters, TargetId target) {
    const std::size_t count = std::min(monsters.size(), flash_left_.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (monsters[i].id() == target) flash_left_[i] = kFlashSeconds;  // 맞은 몬스터를 처음부터 다시 번쩍
    }
}

void MonsterView::set_flash_strength(float strength) {
    flash_strength_ = std::isfinite(strength) ? std::clamp(strength, 0.0f, 1.0f) : 1.0f;
}

float MonsterView::flash_amount(const Monster& monster, std::size_t index) const {
    float amount = std::clamp(flash_left_[index] / kFlashSeconds, 0.0f, 1.0f);  // 맞은 순간 번쩍 (끝으로 흐려짐)
    if (monster.defeated()) {  // 쓰러지는 동안 하얗게 깜빡깜빡 (명세 9.5 사라짐 연출)
        const bool on = std::fmod(monster.state_seconds(), kDeathBlinkPeriod) < kDeathBlinkPeriod * 0.5f;
        amount = std::max(amount, on ? kDeathBlinkAlpha : 0.0f);
    }
    return amount * flash_strength_;  // 접근성 세기 (--flash)
}

void MonsterView::update(float dt_seconds, std::span<const Monster> monsters) {
    const float dt = std::max(0.0f, dt_seconds);
    clock_ += dt;  // 숨쉬기·떨림 시계
    const std::size_t count = std::min(monsters.size(), hop_blend_.size());
    for (std::size_t i = 0; i < count; ++i) {
        // HP 바: 줄면 바로 줄고 꼬리를 남겨요. 번쩍임 시간 줄이기 (11d)
        const auto& monster = monsters[i];
        bars_[i].set(monster.max_hp() > 0 ? static_cast<float>(monster.hp()) / static_cast<float>(monster.max_hp())
                                          : 0.0f);
        bars_[i].update(dt);
        flash_left_[i] = std::max(0.0f, flash_left_[i] - dt);
        // 배회·쫓기·복귀 중이면 통통 뛰어요
        hop_blend_[i] = approach_hop_blend(hop_blend_[i], hops_while(monsters[i].state()), dt);
        // 느낌표 나이: 알아채는 동안 늘고, 아니면 0으로 (다음에 알아채면 처음부터 톡)
        const bool marked = notice_alpha(monsters[i]) > 0.0f;
        notice_age_[i] = marked ? notice_age_[i] + dt : 0.0f;
    }
}

void MonsterView::queue(const engine::Camera2D& camera, std::span<const Monster> monsters, float alpha,
                        bool show_boxes, engine::RenderQueue& queue) const {
    const std::size_t count = std::min(monsters.size(), textures_.size());  // 개수가 다르면 적은 쪽까지만
    for (std::size_t i = 0; i < count; ++i) {
        if (monsters[i].gone()) continue;                    // 다 사라진 몬스터는 그리지 않아요 (11d)
        queue_telegraph_mark(camera, monsters[i], queue);   // 바닥 경고 (몸·그림자 아래)
        queue_monster(camera, monsters[i], i, alpha, queue);
        if (show_boxes) queue_boxes(camera, monsters[i], queue);
    }
}

void MonsterView::queue_monster(const engine::Camera2D& camera, const Monster& monster, std::size_t index,
                                float alpha, engine::RenderQueue& queue) const {
    const engine::Vec2 feet = monster.interpolated_feet(alpha);       // 두 박자 사이 위치
    const float phase = std::fmod(static_cast<float>(index) * kPhaseStep, 1.0f);
    // 싸움 동작(알아채기·예고·덮치기·후딜)이면 그 모양, 아니면 숨쉬기·통통 뛰기
    const MonsterPose pose = slime_action_pose(monster.state(), monster.state_progress(), clock_)
                                 .value_or(slime_pose(clock_, phase, monster.distance_moved(), hop_blend_[index]));
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
    draw.source = {pose.frame * sprite.frame_width, 0, sprite.frame_width, sprite.frame_height};  // 0 보통, 1 웅크림
    draw.dest = {std::round(screen.x - origin_x + pose.offset_x), std::round(screen.y - pose.lift - origin_y),
                 width, height};
    queue.push_sprite(engine::DrawLayer::YSorted, feet.y, draw);  // 주인공·허수아비와 발 위치로 줄 서기
    const float flash = flash_amount(monster, index);
    if (flash > 0.0f) {  // 번쩍임 (11d): 같은 그림을 하얗게 더해요. 줄 서기 값이 같으면 나중 것이 위에 그려져요
        draw.blend = engine::BlendMode::Add;
        draw.tint = {255, 255, 255, static_cast<std::uint8_t>(std::lround(255.0f * flash))};
        queue.push_sprite(engine::DrawLayer::YSorted, feet.y, draw);
    }
    // 머리 위 HP 바 (11d): 맞으면 보이고, 가득 찬 채 2.5초면 숨어요
    queue_enemy_bar(camera, feet, pose.lift + origin_y, bars_[index], queue);
    // 느낌표: 머리 꼭대기(그림 칸 위쪽 끝) 위에
    const float mark = notice_alpha(monster);
    if (mark > 0.0f) {
        const engine::Vec2 head{feet.x, feet.y - pose.lift - origin_y};
        queue_notice_mark(camera, head, notice_age_[index], mark, queue);
    }
}

std::vector<engine::Light> MonsterView::lights(std::span<const Monster> monsters) const {
    std::vector<engine::Light> result;
    for (const auto& monster : monsters) {
        if (const auto shape = telegraph_shape(monster)) {  // 예고 중인 몬스터마다 붉은 빛 하나
            result.push_back(telegraph_light(*shape, monster.state_progress()));
        }
    }
    return result;
}

std::optional<TelegraphShape> MonsterView::telegraph_shape(const Monster& monster) {
    const auto end = monster.lunge_end();          // 예고 중일 때만 있어요
    const combat::AttackData* attack = monster.attack();
    if (!end || attack == nullptr) return std::nullopt;
    // 덮칠 길: 지금 발에서 덮치기 끝까지, 히트박스가 앞으로 나간 만큼 더 (바닥이라 위아래 lift는 빼요)
    const engine::Vec2 ahead = monster.attack_direction() * attack->hitbox.forward;
    return TelegraphShape{monster.feet() + ahead, *end + ahead, attack->hitbox.width};
}

void MonsterView::queue_telegraph_mark(const engine::Camera2D& camera, const Monster& monster,
                                       engine::RenderQueue& queue) const {
    if (const auto shape = telegraph_shape(monster)) {
        queue_telegraph(camera, glow_, *shape, monster.state_progress(), monster.feet().y, queue);
    }
}

void MonsterView::queue_boxes(const engine::Camera2D& camera, const Monster& monster,
                              engine::RenderQueue& queue) const {
    // 상자는 판정과 같은 "지금 박자" 위치에 그려요 (보간하지 않아요: 판정이 실제로 쓰는 자리)
    queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, to_screen(camera, monster.hurtbox()), kHurtboxColor);
    queue.push_fill(engine::DrawLayer::DebugOverlay, 0.0f, to_screen(camera, body_box(monster.feet(), monster.body())),
                    kBodyColor);
    if (const auto hitbox = monster.attack_hitbox()) {  // 덮치는 동안만 빨갛게
        queue.push_fill(engine::DrawLayer::DebugOverlay, 1.0f, to_screen(camera, *hitbox), kHitboxColor);
    }
}

}  // namespace wind::game
