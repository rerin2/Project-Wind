#pragma once

// 이 파일은 무엇을 하나요?
// 물약 마시기 연출이에요. (14d) 보여 주기만 해요 (회복은 PlayerMotion이 꿀꺽 박자에 해요).
// - 마시는 동안 주인공 손에 물약 아이콘이 보이고, 입으로 들며 기울였다가(꿀꺽) 다시 내려요.
// - 꿀꺽하면 몸에서 빛 가루가 피어올라요 (HP 물약은 초록, MP 물약은 파랑)과 잠깐 같은 색 빛.
// - 맞아서 끊기면 병이 바로 사라져요 (가루도 없어요).
// 주인공 그림은 따로 마시는 그림이 없어서 서기·걷기 그림 그대로예요 (아이콘 병이 동작을 보여 줘요).

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/presentation/loot_view_setup.hpp"

#include "wind/items/item_def.hpp"
#include "wind/world/direction.hpp"

#include <cstdint>
#include <optional>

namespace wind::game {

class PlayerMotion;

class DrinkView {
public:
    explicit DrinkView(const LootViewSetup& setup) : setup_(setup) {}

    // 마시기가 시작됐어요: 이 아이콘의 병을 들고, 효과 색(HP 초록, MP 파랑)을 기억해요
    void start(std::int32_t icon, const items::UseEffect& effect);
    // 화면 한 장마다: 주인공이 아직 마시는 중인지 보고, 꿀꺽한 박자면 빛 가루를 뿌려요. feet: 그린 발 위치
    void update(float dt_seconds, const PlayerMotion& hero, engine::Vec2 feet, engine::ParticleRandom& random,
                engine::ParticlePool& air_fx);
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const;
    // 꿀꺽 직후 잠깐 켜지는 빛 (없으면 nullopt)
    [[nodiscard]] std::optional<engine::Light> light() const;

    // 시작한 뒤 흐른 시간으로 정한 병 기울기 (도, 0 = 똑바로, 음수 = 입 쪽으로). 테스트용으로도 열어 둬요
    [[nodiscard]] static float tilt_degrees(float elapsed);
    [[nodiscard]] bool showing() const { return showing_; }

private:
    LootViewSetup setup_;
    bool showing_ = false;          // 병이 보이는 중
    std::int32_t icon_ = 0;         // 병 아이콘 칸
    bool mana_ = false;             // MP 물약인가 (색)
    float elapsed_ = 0.0f;          // 마시기 시작한 뒤 시간 (주인공 시간표에서 읽어요)
    engine::Vec2 feet_;             // 그린 발 위치
    world::Direction facing_ = world::Direction::Down;
    float flash_ = 0.0f;            // 꿀꺽 빛이 남은 시간
    bool seen_gulp_ = false;        // 이번 마시기에서 꿀꺽 가루를 이미 뿌렸나
};

}  // namespace wind::game
