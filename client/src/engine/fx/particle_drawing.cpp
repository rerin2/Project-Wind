// 이 파일은 무엇을 하나요?
// 파티클 점들을 화면 위치로 바꿔서 그리기 목록에 넣는 곳이에요.

#include "engine/fx/particle_drawing.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::engine {
namespace {

std::uint8_t mix(std::uint8_t a, std::uint8_t b, float t) {
    return static_cast<std::uint8_t>(std::lround(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t));
}

}  // namespace

Color particle_color(const Particle& p) {
    const float t = std::clamp(p.progress(), 0.0f, 1.0f);
    Color c{mix(p.start_color.r, p.end_color.r, t), mix(p.start_color.g, p.end_color.g, t),
            mix(p.start_color.b, p.end_color.b, t), mix(p.start_color.a, p.end_color.a, t)};
    // 처음 fade_in 구간에서는 0에서 서서히 진해지고,
    // 마지막 fade_out 구간에서는 서서히 0으로 흐려져요. 그 사이는 원래 진하기 그대로예요.
    float strength = 1.0f;
    if (p.fade_in > 0.0f && t < p.fade_in) {
        strength = std::min(strength, t / p.fade_in);
    }
    if (p.fade_out > 0.0f && t > 1.0f - p.fade_out) {
        strength = std::min(strength, (1.0f - t) / p.fade_out);
    }
    c.a = static_cast<std::uint8_t>(static_cast<float>(c.a) * std::clamp(strength, 0.0f, 1.0f));
    return c;
}

float particle_wobble(const Particle& p) {
    if (p.wobble_amplitude <= 0.0f) {
        return 0.0f;
    }
    const float turn = 2.0f * std::numbers::pi_v<float> * p.wobble_frequency * p.age;
    return p.wobble_amplitude * std::sin(turn + p.wobble_phase);
}

std::uint32_t queue_particles(const ParticlePool& pool, const Camera2D& camera,
                              const ParticleTextures& textures, DrawLayer layer,
                              RenderQueue& queue) {
    const RectF view = camera.view_rect();
    std::uint32_t queued = 0;
    for (const Particle& p : pool.particles()) {
        const TextureHandle texture = p.shape == ParticleShape::Pixel ? textures.pixel : textures.soft;
        if (!texture.valid()) {
            continue;  // 그림을 못 만들었으면 그 모양은 건너뛰어요
        }
        const float t = std::clamp(p.progress(), 0.0f, 1.0f);
        // 크기는 수명에 따라 시작 크기에서 끝 크기로 점점 바뀌어요.
        const float size = p.start_size + (p.end_size - p.start_size) * t;
        if (size < 0.5f) {
            continue;  // 너무 작으면 안 보이니까 건너뛰어요
        }
        const Color color = particle_color(p);
        if (color.a == 0) {
            continue;  // 완전히 투명하면 그릴 필요가 없어요
        }
        const Vec2 center{p.position.x + particle_wobble(p), p.position.y};
        const RectF world_box{center.x - size * 0.5f, center.y - size * 0.5f, size, size};
        if (!world_box.intersects(view)) {
            continue;  // 화면 밖이에요
        }
        const Vec2 screen = camera.world_to_screen({world_box.x, world_box.y});
        SpriteDraw sprite;
        sprite.texture = texture;
        sprite.source = {0, 0, 0, 0};  // 0 크기 = 그림 전체를 써요
        // 도트 느낌을 살리려고 위치와 크기를 정수 픽셀로 맞춰요.
        const float pixel_size = std::max(1.0f, std::round(size));
        sprite.dest = {std::round(screen.x), std::round(screen.y), pixel_size, pixel_size};
        sprite.tint = color;
        sprite.blend = p.blend;
        queue.push_sprite(layer, p.position.y, sprite);
        ++queued;
    }
    return queued;
}

}  // namespace wind::engine
