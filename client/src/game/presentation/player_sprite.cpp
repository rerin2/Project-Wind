// 이 파일은 무엇을 하나요?
// 주인공 그림을 넘기고, 잔상을 남기고, 그리기 목록에 넣는 곳이에요.

#include "game/presentation/player_sprite.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string_view>
#include <utility>

namespace wind::game {
namespace {

// 그림 설명서에 꼭 있어야 하는 동작 그림(클립) 이름
constexpr std::string_view kIdleClip = "idle";  // 가만히 서 있기
constexpr std::string_view kWalkClip = "walk";  // 걷기 (달리기도 이 그림을 더 빨리 넘겨요)
// 쿠션 쉬기 그림 (13b, 선택: 없으면 서 있기 그림). 앉고 나서 1.4초 뒤 웅크려요.
constexpr std::string_view kSitClip = "sit";
constexpr std::string_view kCurlClip = "curl";
constexpr std::string_view kStretchClip = "stretch";
constexpr float kCurlAfterSeconds = 1.4f;
// 걷기 그림 넘기는 속도: 최고 걷기 속도일 때 1배, 느리면 천천히 (최소 0.5배),
// 달리면 속도만큼 빨리 (1.5배), 구를 때는 2배
constexpr double kMinWalkAnimRate = 0.5;
constexpr double kDodgeAnimRate = 2.0;
// 쓰러짐(12b): 다 누우면 90도 돌아가고, 그림 가운데가 이만큼 내려가요 (픽셀).
// 그림 가운데는 발보다 22픽셀 위라서, 14픽셀 내리면 누운 몸이 발 높이 바닥에 닿아요.
// 이유는 docs/VISUALS.md "Hero falling" 절에 있어요.
constexpr float kLyingDegrees = 90.0f;
constexpr float kLyingDrop = 14.0f;

}  // namespace

PlayerSprite::PlayerSprite(engine::SpriteSheet sheet, engine::TextureHandle texture)
    : sheet_(std::move(sheet)), texture_(texture) {
    animator_.play(sheet_.clip(kIdleClip));  // 처음엔 서 있는 모습
}

bool PlayerSprite::play_rest_clip(const HeroRest& rest) {
    std::string_view name;
    switch (rest.phase()) {
        case HeroRest::Phase::None: return false;
        case HeroRest::Phase::Settling: name = kSitClip; break;   // 앉는 중
        case HeroRest::Phase::Sitting:                            // 잠깐 앉아 있다가 웅크려 졸아요
            name = rest.phase_seconds() < kCurlAfterSeconds ? kSitClip : kCurlClip;
            break;
        case HeroRest::Phase::Standing: name = kStretchClip; break;  // 기지개
    }
    const engine::AnimationClip* clip = sheet_.clip(name);
    animator_.play(clip != nullptr ? clip : sheet_.clip(kIdleClip));  // 그림이 없으면 서 있기
    return true;
}

void PlayerSprite::update(double dt_seconds, const Mover& mover, engine::Vec2 feet, const HeroRest& rest) {
    double rate = 1.0;
    if (play_rest_clip(rest)) {  // 쿠션에서 쉬는 중 (13b): 다리 상태보다 먼저
        animator_.update(dt_seconds);
        trail_.update(static_cast<float>(dt_seconds), false, feet, sheet_.frame_rect(mover.facing(), 0));
        return;
    }
    switch (mover.state()) {
        case Mover::State::Idle:
            animator_.play(sheet_.clip(kIdleClip));
            break;
        case Mover::State::Walking:
        case Mover::State::Running: {
            animator_.play(sheet_.clip(kWalkClip));
            // 천천히 걸으면 발도 천천히 움직여요 (발이 미끄러지는 것처럼 보이지 않게)
            const double max_speed = mover.tuning().max_speed;
            rate = max_speed > 0.0 ? std::max(kMinWalkAnimRate, mover.speed() / max_speed) : 1.0;
            break;
        }
        case Mover::State::Dodging:
            // 구르기 그림은 아직 없어서, 걷기 그림을 두 배 빨리 넘겨요.
            animator_.play(sheet_.clip(kWalkClip));
            rate = kDodgeAnimRate;
            break;
    }
    animator_.update(dt_seconds * rate);

    // 잔상: 구르는 동안 지금 그림을 사진으로 찍어 둬요
    trail_.update(static_cast<float>(dt_seconds), mover.state() == Mover::State::Dodging, feet,
                  sheet_.frame_rect(animator_.clip(), mover.facing(), animator_.column()));
}

void PlayerSprite::queue(const engine::Camera2D& camera, engine::Vec2 feet, world::Direction facing,
                         engine::RenderQueue& queue, float alpha, float lying) const {
    trail_.queue(camera, texture_, sheet_.origin, {sheet_.frame_width, sheet_.frame_height}, queue);

    // 그림 속 "발 위치"(origin)가 맵 위의 진짜 발 위치에 딱 오도록 그려요.
    const engine::Vec2 screen = camera.world_to_screen(feet);  // 맵 위치 -> 화면 위치
    engine::SpriteDraw sprite;
    sprite.texture = texture_;
    // 그림 묶음에서 "지금 방향, 지금 동작 장면"에 해당하는 부분을 오려 와요.
    sprite.source = sheet_.frame_rect(animator_.clip(), facing, animator_.column());  // 쉬기 그림은 정해진 행
    // 위치는 반올림해서 정수 픽셀에 맞춰요. 그래야 픽셀 그림이 흐릿하게 떨리지 않아요.
    sprite.dest = {std::round(screen.x - static_cast<float>(sheet_.origin.x)),
                   std::round(screen.y - static_cast<float>(sheet_.origin.y)),
                   static_cast<float>(sheet_.frame_width), static_cast<float>(sheet_.frame_height)};
    // 투명도: 맞은 뒤 무적이면 깜빡여요 (11c, hero_blink). 1이면 그대로
    sprite.tint.a = static_cast<std::uint8_t>(std::lround(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
    // 쓰러짐(12b): 그림 가운데를 축으로 돌리고, 누운 몸이 바닥에 닿게 내려요. 보여 주기만 해요.
    const float lie = std::isfinite(lying) ? std::clamp(lying, 0.0f, 1.0f) : 0.0f;
    if (lie > 0.0f) {
        const float side = facing == world::Direction::Left ? -1.0f : 1.0f;  // 왼쪽을 보면 왼쪽으로 쓰러져요
        sprite.rotation_degrees = side * kLyingDegrees * lie;
        sprite.dest.y += std::round(kLyingDrop * lie);                     // 정수 픽셀로 내려요
    }
    // 발 위치(y)로 줄을 세워서, 나무 밑동보다 아래(앞)에 서면 나무보다 앞에 그려져요.
    queue.push_sprite(engine::DrawLayer::YSorted, feet.y, sprite);
}

}  // namespace wind::game
