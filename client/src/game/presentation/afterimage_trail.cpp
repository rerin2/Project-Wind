// 이 파일은 무엇을 하나요?
// 잔상 사진을 찍고, 흐리게 만들고, 그리기 목록에 넣는 곳이에요.

#include "game/presentation/afterimage_trail.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wind::game {
namespace {

constexpr float kSnapshotEvery = 0.035f;  // 몇 초마다 사진을 찍을지
constexpr float kFadeSeconds = 0.18f;     // 사진이 사라질 때까지 걸리는 시간
constexpr float kStartAlpha = 120.0f;     // 막 찍은 사진의 진하기 (255가 완전 진함)

}  // namespace

void AfterimageTrail::update(float dt_seconds, bool recording, engine::Vec2 feet,
                             engine::RectI source) {
    for (auto& s : snapshots_) {
        s.age += dt_seconds;
    }
    std::erase_if(snapshots_, [](const Snapshot& s) { return s.age >= kFadeSeconds; });

    if (!recording) {
        since_last_ = kSnapshotEvery;  // 다음에 구르기 시작하면 바로 한 장 찍어요
        return;
    }
    since_last_ += dt_seconds;
    if (since_last_ >= kSnapshotEvery) {
        since_last_ = 0.0f;
        snapshots_.push_back({feet, source, 0.0f});
    }
}

void AfterimageTrail::queue(const engine::Camera2D& camera, engine::TextureHandle texture,
                            engine::Vec2i origin, engine::Vec2i frame_size,
                            engine::RenderQueue& queue) const {
    for (const auto& s : snapshots_) {
        const engine::Vec2 screen = camera.world_to_screen(s.feet);
        engine::SpriteDraw sprite;
        sprite.texture = texture;
        sprite.source = s.source;
        sprite.dest = {std::round(screen.x - static_cast<float>(origin.x)),
                       std::round(screen.y - static_cast<float>(origin.y)),
                       static_cast<float>(frame_size.x), static_cast<float>(frame_size.y)};
        const float fade = 1.0f - std::clamp(s.age / kFadeSeconds, 0.0f, 1.0f);
        sprite.tint.a = static_cast<std::uint8_t>(kStartAlpha * fade);
        // 진짜 캐릭터보다 아주 살짝 위(뒤)에 줄을 세워서, 캐릭터가 잔상 앞에 보이게 해요.
        queue.push_sprite(engine::DrawLayer::YSorted, s.feet.y - 0.01f, sprite);
    }
}

}  // namespace wind::game
