// 이 파일은 무엇을 하나요?
// 바닥 아이템 연출: 톡 튀어 떨어지기, 둥실거리기, 반짝임, 밤 빛, 주인공에게 빨려 들기.
// 수치의 이유는 docs/VISUALS.md "Ground items" 절에 있어요.

#include "game/presentation/loot_view.hpp"

#include "engine/fx/particle_spawn.hpp"
#include "game/items/map_loot.hpp"
#include "game/presentation/rarity_color.hpp"
#include "game/presentation/shadow_drawing.hpp"

#include "wind/items/item_catalog.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::game {
namespace {

constexpr float kHopSeconds = 0.45f;     // 첫 포물선 시간 (쓰러진 자리 → 떨어질 자리)
constexpr float kHopHeight = 14.0f;      // 첫 포물선 꼭대기 높이 (픽셀)
constexpr float kBounceSeconds = 0.15f;  // 땅에 닿고 한 번 더 작게 튀는 시간 (합 0.6초 = 줍기 가능 시간)
constexpr float kBounceHeight = 4.0f;    // 작게 튀는 높이
constexpr float kBobHeight = 1.5f;       // 바닥에서 둥실거리는 높이
constexpr float kBobSpeed = 2.4f;        // 둥실 빠르기 (라디안/초)
constexpr float kIconLift = 8.0f;        // 아이콘 가운데는 땅에서 이만큼 위 (16픽셀 아이콘의 반)
constexpr float kGlintPeriod = 2.2f;     // 이만큼마다 한 번 반짝 (초)
constexpr float kGlintSeconds = 0.3f;    // 반짝이는 시간
constexpr float kGlintSize = 9.0f;       // 반짝임 가장 클 때 크기 (픽셀)
constexpr engine::Vec2 kGlintOffset{5.0f, -5.0f};  // 아이콘 오른쪽 위 모서리쯤
constexpr engine::Vec2 kShadowSize{12.0f, 4.0f};   // 땅에 있을 때 그림자 크기
constexpr float kShadowShrink = 0.3f;    // 1픽셀 뜰 때마다 그림자 가로가 줄어드는 양
constexpr float kFlySeconds = 0.28f;     // 주인공에게 빨려 드는 시간
constexpr float kFlyEndScale = 0.5f;     // 닿을 때 크기 (처음의 반)
constexpr float kLightRadius = 18.0f;    // 밤 빛 크기
constexpr float kLightIntensity = 0.55f; // 밤 빛 밝기

}  // namespace

LootView::LootView(LootViewSetup setup, const items::ItemCatalog& catalog) : setup_(setup), catalog_(catalog) {}

float LootView::hop_height(float age) {
    if (age < 0.0f) return 0.0f;
    if (age < kHopSeconds) {
        const float u = age / kHopSeconds;    // 0 → 1
        return 4.0f * kHopHeight * u * (1.0f - u);  // 포물선: 가운데서 가장 높아요
    }
    if (age < kHopSeconds + kBounceSeconds) {
        const float u = (age - kHopSeconds) / kBounceSeconds;
        return 4.0f * kBounceHeight * u * (1.0f - u);  // 작게 한 번 더
    }
    return 0.0f;
}

engine::Vec2 LootView::hop_ground(const GroundItem& item) {
    const float u = std::clamp(item.age / kHopSeconds, 0.0f, 1.0f);
    const float eased = 1.0f - (1.0f - u) * (1.0f - u);  // 처음엔 빠르고 끝에서 느려져요
    return item.from + (item.at - item.from) * eased;
}

int LootView::icon_of(const GroundItem& item) const {
    const auto* def = catalog_.find(item.stack.item_id);
    return def != nullptr ? def->icon : 0;
}

void LootView::collect(const std::vector<GroundItem>& picked) {
    pending_.insert(pending_.end(), picked.begin(), picked.end());
}

void LootView::update(float dt_seconds, engine::Vec2 body, engine::ParticleRandom& random,
                      engine::ParticlePool& air_fx) {
    clock_ += dt_seconds;
    for (const auto& item : pending_) {  // 주운 것: 바닥 자리에서 빨려 들기 시작
        const engine::Vec2 start{item.at.x, item.at.y - kIconLift};
        flying_.push_back({icon_of(item), start, start, 0.0f});
    }
    pending_.clear();
    for (auto& fly : flying_) {
        fly.age += dt_seconds;
        const float u = std::min(1.0f, fly.age / kFlySeconds);
        fly.position = fly.start + (body - fly.start) * (u * u);  // 처음엔 느리고 점점 빨라져요 (빨려 듦)
        if (u >= 1.0f && setup_.pickup_fx != nullptr) {
            engine::emit_burst(*setup_.pickup_fx, body, setup_.pickup_fx->direction, random, air_fx);  // 닿으면 반짝
        }
    }
    std::erase_if(flying_, [](const Flying& fly) { return fly.age >= kFlySeconds; });
}

void LootView::queue_icon(const engine::Camera2D& camera, int icon, engine::Vec2 center, float scale,
                          float sort_y, engine::RenderQueue& queue) const {
    const float size = static_cast<float>(kIconSize) * scale;
    const engine::Vec2 screen = camera.world_to_screen(center);
    engine::SpriteDraw sprite;
    sprite.texture = setup_.icons;
    sprite.source = {icon * kIconSize, 0, kIconSize, kIconSize};  // 그림 판에서 이 아이템 칸
    sprite.dest = {std::round(screen.x - size * 0.5f), std::round(screen.y - size * 0.5f), size, size};
    queue.push_sprite(engine::DrawLayer::YSorted, sort_y, sprite);
}

void LootView::queue(const engine::Camera2D& camera, const MapLoot& loot, engine::RenderQueue& queue) const {
    for (const auto& item : loot.items()) {
        const engine::Vec2 ground = hop_ground(item);
        float height = hop_height(item.age);
        // 다 떨어졌으면 둥실둥실 (아이템마다 박자가 조금씩 달라요)
        if (item.age >= kHopSeconds + kBounceSeconds) {
            height = kBobHeight * (0.5f + 0.5f * std::sin(clock_ * kBobSpeed + static_cast<float>(item.id) * 1.3f));
        }
        // 그림자: 높이 뜰수록 작아져요
        const engine::Vec2 shadow{std::max(4.0f, kShadowSize.x - height * kShadowShrink), kShadowSize.y};
        queue_shadow(camera, setup_.shadow, ground, shadow, queue);
        const engine::Vec2 center{ground.x, ground.y - kIconLift - std::round(height)};
        queue_icon(camera, icon_of(item), center, 1.0f, ground.y, queue);
        // 반짝: 아이템마다 다른 때에 잠깐 커졌다 작아지는 빛 (바닥에 닿은 뒤에만)
        const float phase = std::fmod(clock_ + static_cast<float>(item.id) * 0.37f, kGlintPeriod);
        if (item.age >= kHopSeconds && phase < kGlintSeconds) {
            const float size = kGlintSize * std::sin(std::numbers::pi_v<float> * phase / kGlintSeconds);
            const engine::Vec2 screen = camera.world_to_screen(center + kGlintOffset);
            engine::SpriteDraw glint;
            glint.texture = setup_.glow;
            glint.dest = {std::round(screen.x - size * 0.5f), std::round(screen.y - size * 0.5f), size, size};
            glint.blend = engine::BlendMode::Add;
            queue.push_sprite(engine::DrawLayer::Glow, ground.y, glint);
        }
    }
    for (const auto& fly : flying_) {  // 빨려 드는 아이콘은 나뭇잎보다 위에
        const float u = std::min(1.0f, fly.age / kFlySeconds);
        const float scale = 1.0f + (kFlyEndScale - 1.0f) * u;
        const float size = static_cast<float>(kIconSize) * scale;
        const engine::Vec2 screen = camera.world_to_screen(fly.position);
        engine::SpriteDraw sprite;
        sprite.texture = setup_.icons;
        sprite.source = {fly.icon * kIconSize, 0, kIconSize, kIconSize};
        sprite.dest = {std::round(screen.x - size * 0.5f), std::round(screen.y - size * 0.5f), size, size};
        queue.push_sprite(engine::DrawLayer::WorldFx, fly.position.y, sprite);
    }
}

std::vector<engine::Light> LootView::lights(const MapLoot& loot) const {
    std::vector<engine::Light> out;
    for (const auto& item : loot.items()) {
        const auto* def = catalog_.find(item.stack.item_id);
        engine::Light light;
        light.position = hop_ground(item) - engine::Vec2{0.0f, kIconLift};
        light.radius = kLightRadius;
        light.color = rarity_color(def != nullptr ? def->rarity : items::Rarity::Common);
        light.intensity = kLightIntensity;
        light.pulse_frequency = 0.5f;  // 천천히 숨쉬듯
        light.pulse_depth = 0.25f;
        light.phase = static_cast<float>(item.id) * 0.7f;  // 아이템마다 다른 박자
        out.push_back(light);
    }
    return out;
}

}  // namespace wind::game
