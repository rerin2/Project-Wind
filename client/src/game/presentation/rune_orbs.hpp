#pragma once

// 이 파일은 무엇을 하나요?
// 룬이 주인공에게 빨려 드는 연출이에요. (명세 8.3, 13c) 보여 주기만 해요 (룬은 이미 지갑에 있어요).
// - 몬스터가 쓰러진 자리에서 금빛 알갱이가 사방으로 톡 튀어나와요 (룬 양에 따라 3~10개)
// - 잠깐(0.3초) 뒤부터 주인공 쪽으로 점점 빨라지며 날아가요
// - 주인공 몸에 닿으면 사라져요. 2초가 넘어도 못 닿으면(멀리 도망감) 그냥 사라져요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/render/render_queue.hpp"

#include <cstdint>
#include <vector>

namespace wind::game {

class RuneOrbs {
public:
    // at(맵 위 픽셀)에서 amount 룬만큼 알갱이를 만들어요
    void spawn(engine::Vec2 at, std::int64_t amount, engine::ParticleRandom& random);
    // 화면 한 장마다: target(주인공 몸 가운데)으로 날아가요
    void update(float dt_seconds, engine::Vec2 target);
    // glow: 둥근 빛 그림 (더하기 섞기로 반짝여요)
    void queue(const engine::Camera2D& camera, engine::TextureHandle glow, engine::RenderQueue& queue) const;

    [[nodiscard]] std::size_t count() const { return orbs_.size(); }

private:
    struct Orb {
        engine::Vec2 position;
        engine::Vec2 velocity;  // 픽셀/초
        float age = 0.0f;
    };
    std::vector<Orb> orbs_;
};

}  // namespace wind::game
