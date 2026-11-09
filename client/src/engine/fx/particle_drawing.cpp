// 이 파일은 무엇을 하나요?
// 파티클 점들을 화면 위치로 바꿔서 그리기 목록에 넣는 곳이에요.

#include "engine/fx/particle_drawing.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {

std::uint32_t queue_particles(const ParticlePool& pool, const Camera2D& camera,
                              TextureHandle texture, DrawLayer layer, RenderQueue& queue) {
    const RectF view = camera.view_rect();
    std::uint32_t queued = 0;
    for (const Particle& p : pool.particles()) {
        const float t = std::clamp(p.progress(), 0.0f, 1.0f);
        // 크기와 진하기는 수명에 따라 시작값에서 끝값으로 점점 바뀌어요.
        const float size = p.start_size + (p.end_size - p.start_size) * t;
        if (size < 0.5f) {
            continue;  // 너무 작으면 안 보이니까 건너뛰어요
        }
        const RectF world_box{p.position.x - size * 0.5f, p.position.y - size * 0.5f, size, size};
        if (!world_box.intersects(view)) {
            continue;  // 화면 밖이에요
        }
        const Vec2 screen = camera.world_to_screen({world_box.x, world_box.y});
        SpriteDraw sprite;
        sprite.texture = texture;
        sprite.source = {0, 0, 0, 0};  // 0 크기 = 그림 전체를 써요 (아래에서 채워요)
        // 도트 느낌을 살리려고 위치와 크기를 정수 픽셀로 맞춰요.
        const float pixel_size = std::max(1.0f, std::round(size));
        sprite.dest = {std::round(screen.x), std::round(screen.y), pixel_size, pixel_size};
        sprite.tint.a = static_cast<std::uint8_t>(static_cast<float>(p.start_alpha) * (1.0f - t));
        queue.push_sprite(layer, p.position.y, sprite);
        ++queued;
    }
    return queued;
}

}  // namespace wind::engine
