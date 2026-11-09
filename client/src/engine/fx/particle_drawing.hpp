#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 통에 든 점들을 "그리기 목록"(렌더 큐)에 넣는 도우미예요.
// 점마다 수명에 맞춰 크기를 줄이고 흐리게 만들어요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/render/render_queue.hpp"

#include <cstdint>

namespace wind::engine {

// 화면에 보이는 점만 넣고, 넣은 개수를 돌려줘요.
// texture: 점 하나에 쓸 그림 (부드러운 동그라미 등)
std::uint32_t queue_particles(const ParticlePool& pool, const Camera2D& camera,
                              TextureHandle texture, DrawLayer layer, RenderQueue& queue);

}  // namespace wind::engine
