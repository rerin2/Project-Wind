#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 통에 든 점들을 "그리기 목록"(렌더 큐)에 넣는 도우미예요.
// 점마다 수명에 맞춰 크기와 색을 바꾸고, 처음엔 서서히 나타나고, 살랑살랑 흔들리게 해요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/render/render_queue.hpp"

#include <cstdint>
#include <optional>

namespace wind::engine {

// 점 모양마다 쓸 그림. 둘 다 흰색이라 점의 색을 곱해서 원하는 색을 만들어요.
struct ParticleTextures {
    TextureHandle soft;   // 부드러운 동그라미
    TextureHandle pixel;  // 또렷한 흰 네모
};

// 화면에 보이는 점만 넣고, 넣은 개수를 돌려줘요.
// glow_layer가 있으면 "더하기" 섞기 점(스스로 빛나는 것)은 그 단계로 보내요.
std::uint32_t queue_particles(const ParticlePool& pool, const Camera2D& camera,
                              const ParticleTextures& textures, DrawLayer layer,
                              RenderQueue& queue, std::optional<DrawLayer> glow_layer = std::nullopt);

// 수명 진행률 t(0~1)일 때 점의 색 (fade_in 반영). 테스트하기 쉽게 따로 꺼내 뒀어요.
[[nodiscard]] Color particle_color(const Particle& particle);

// 지금 살랑임 때문에 옆으로 얼마나 밀렸는지 (픽셀)
[[nodiscard]] float particle_wobble(const Particle& particle);

}  // namespace wind::engine
