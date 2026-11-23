// 이 파일은 무엇을 하나요?
// 물약 마시기 연출: 손에 든 병 들기·기울이기·내리기, 꿀꺽 빛 가루와 빛.
// 수치의 이유는 docs/VISUALS.md "Drinking" 절에 있어요.

#include "game/presentation/drink_view.hpp"

#include "engine/fx/particle_spawn.hpp"
#include "engine/tween/easing.hpp"
#include "game/entities/player_motion.hpp"
#include "game/entities/potion_drinker.hpp"
#include "game/presentation/loot_view.hpp"

#include <algorithm>
#include <cmath>

namespace wind::game {
namespace {

constexpr float kTiltMax = 70.0f;        // 꿀꺽할 때 가장 많이 기울인 각도 (도)
constexpr float kHandSide = 7.0f;        // 옆을 볼 때 손이 몸에서 떨어진 거리 (픽셀)
constexpr float kHandFront = 5.0f;       // 위·아래를 볼 때 손 자리 (몸 가운데에서 오른쪽)
constexpr float kHandHeight = 14.0f;     // 발에서 손까지 높이
constexpr float kLift = 6.0f;            // 입으로 가져갈 때 더 올라가는 높이
constexpr float kBodyLift = 12.0f;       // 빛 가루가 나오는 몸 가운데 (발에서 위로)
constexpr float kFlashSeconds = 0.35f;   // 꿀꺽 빛이 보이는 시간
constexpr float kFlashRadius = 44.0f;    // 꿀꺽 빛 크기
constexpr engine::Color kHealColor{120, 230, 140, 255};  // HP 물약 빛 (초록)
constexpr engine::Color kManaColor{110, 150, 255, 255};  // MP 물약 빛 (파랑)

}  // namespace

float DrinkView::tilt_degrees(float elapsed) {
    const float gulp = PotionDrinker::kGulpSeconds;
    const float total = PotionDrinker::kDrinkSeconds;
    if (elapsed <= 0.0f) return 0.0f;
    if (elapsed < gulp) return -kTiltMax * engine::ease(engine::Ease::InOutQuad, elapsed / gulp);  // 들어 올리며 기울여요
    const float down = std::clamp((elapsed - gulp) / (total - gulp), 0.0f, 1.0f);
    return -kTiltMax * (1.0f - engine::ease(engine::Ease::OutQuad, down));  // 꿀꺽 뒤 내려요
}

void DrinkView::start(std::int32_t icon, const items::UseEffect& effect) {
    showing_ = true;
    icon_ = icon;
    mana_ = effect.heal_hp <= 0 && effect.heal_mp > 0;  // MP만 채우는 물약이면 파랑
    elapsed_ = 0.0f;
    seen_gulp_ = false;
}

void DrinkView::update(float dt_seconds, const PlayerMotion& hero, engine::Vec2 feet, engine::ParticleRandom& random,
                       engine::ParticlePool& air_fx) {
    flash_ = std::max(0.0f, flash_ - dt_seconds);
    const auto& drinker = hero.drinker();
    feet_ = feet;
    facing_ = hero.mover().facing();
    // 꿀꺽 (시간표에서 한 번만 잡아요: 한 화면에 박자가 여러 번이어도 놓치지 않게)
    if (showing_ && drinker.gulped() && !seen_gulp_) {
        seen_gulp_ = true;
        flash_ = kFlashSeconds;
        const auto* fx = mana_ ? setup_.mana_fx : setup_.heal_fx;
        if (fx != nullptr) engine::emit_burst(*fx, feet - engine::Vec2{0.0f, kBodyLift}, fx->direction, random, air_fx);
    }
    if (!drinker.busy()) {  // 다 마셨거나 맞아서 끊겼어요
        showing_ = false;
        return;
    }
    elapsed_ = drinker.elapsed();
}

void DrinkView::queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const {
    if (!showing_) return;
    const float tilt = tilt_degrees(elapsed_);
    const float raise = kLift * (-tilt / kTiltMax);  // 많이 기울일수록 입 쪽으로 올라가요
    float side = kHandFront;
    if (facing_ == world::Direction::Left) side = -kHandSide;
    if (facing_ == world::Direction::Right) side = kHandSide;
    const engine::Vec2 hand{feet_.x + side, feet_.y - kHandHeight - raise};
    const float size = static_cast<float>(LootView::kIconSize);
    const engine::Vec2 screen = camera.world_to_screen(hand);
    engine::SpriteDraw sprite;
    sprite.texture = setup_.icons;
    sprite.source = {icon_ * LootView::kIconSize, 0, LootView::kIconSize, LootView::kIconSize};
    sprite.dest = {std::round(screen.x - size * 0.5f), std::round(screen.y - size * 0.5f), size, size};
    sprite.rotation_degrees = facing_ == world::Direction::Left ? -tilt : tilt;  // 왼쪽을 보면 반대로 기울여요
    // 위를 볼 때는 몸 뒤에, 나머지는 몸 앞에 (같은 발 줄에서 순서만 조금 바꿔요)
    const float sort_y = facing_ == world::Direction::Up ? feet_.y - 0.5f : feet_.y + 0.5f;
    queue.push_sprite(engine::DrawLayer::YSorted, sort_y, sprite);
}

std::optional<engine::Light> DrinkView::light() const {
    if (flash_ <= 0.0f) return std::nullopt;
    engine::Light light;
    light.position = feet_ - engine::Vec2{0.0f, kBodyLift};
    light.radius = kFlashRadius;
    light.color = mana_ ? kManaColor : kHealColor;
    light.intensity = flash_ / kFlashSeconds;  // 꿀꺽 순간 가장 밝고 점점 어두워져요
    return light;
}

}  // namespace wind::game
