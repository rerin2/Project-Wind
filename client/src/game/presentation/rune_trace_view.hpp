#pragma once

// 이 파일은 무엇을 하나요?
// 룬 흔적 연출이에요. (명세 8.3.1, 14e) 보여 주기만 해요 (되찾기는 FieldRunes가 정해요).
// - 흔적: 땅에 금빛 웅덩이가 숨쉬듯 밝아졌다 어두워지고, 작은 금빛 점 세 개가 천천히 둘레를 돌고,
//   위로 금빛 가루가 조금씩 피어올라요. 밤에도 보이게 금빛 빛을 내요.
// - 되찾으면: 흔적 자리에서 금빛 알갱이가 튀어나와 주인공 몸으로 빨려 들어요 (13c 룬 알갱이와 같은 모양).
// 이 맵의 흔적만 그려요 (다른 맵에 있으면 아무것도 안 보여요).

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/presentation/rune_orbs.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace wind::game {

class RuneTraceView {
public:
    // glow: 둥근 빛 그림, motes: 피어오르는 금빛 가루 효과 (없으면 가루 없이)
    RuneTraceView(engine::TextureHandle glow, const engine::ParticleEffect* motes) : glow_(glow), motes_(motes) {}

    // 화면 한 장마다. trace_at: 이 맵에 흔적이 있으면 그 자리 (없으면 nullopt), body: 주인공 몸 가운데
    void update(float dt_seconds, std::optional<engine::Vec2> trace_at, engine::Vec2 body,
                engine::ParticleRandom& random, engine::ParticlePool& air_fx);
    // 되찾았어요: 그 자리에서 amount 룬만큼 알갱이가 튀어나와 주인공에게 빨려 들어요
    void recovered(engine::Vec2 at, std::int64_t amount, engine::ParticleRandom& random);
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const;
    // 흔적의 금빛 빛 (없으면 빈 목록)
    [[nodiscard]] std::vector<engine::Light> lights() const;

    [[nodiscard]] bool showing() const { return at_.has_value(); }
    [[nodiscard]] std::size_t flying_orbs() const { return orbs_.count(); }

private:
    engine::TextureHandle glow_;
    const engine::ParticleEffect* motes_;
    std::optional<engine::Vec2> at_;  // 지금 보이는 흔적 자리
    RuneOrbs orbs_;                   // 되찾을 때 빨려 드는 알갱이
    float clock_ = 0.0f;              // 숨쉬기·돌기 시계
    float emit_ = 0.0f;               // 다음 가루까지 남은 시간
};

}  // namespace wind::game
