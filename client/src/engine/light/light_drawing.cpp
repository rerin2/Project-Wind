// 이 파일은 무엇을 하나요?
// 빛 하나하나를 화면 위치·크기·밝기로 바꿔 "더하기" 그리기 명령으로 만드는 곳이에요.

#include "engine/light/light_drawing.hpp"

#include "engine/light/light_animation.hpp"

#include <cmath>
#include <cstdint>

namespace wind::engine {

LightStats queue_lights(const std::vector<Light>& lights, const Camera2D& camera,
                        double time_seconds, TextureHandle texture, LightingFrame& frame) {
    LightStats stats;
    stats.total = static_cast<std::uint32_t>(lights.size());
    const RectF view = camera.view_rect();
    for (const Light& light : lights) {
        const float radius = light.radius * light_radius_scale(light, time_seconds);
        const RectF box{light.position.x - radius, light.position.y - radius, radius * 2.0f,
                        radius * 2.0f};
        if (!box.intersects(view)) {
            ++stats.offscreen;  // 빛이 화면에 한 픽셀도 안 닿아요
            continue;
        }
        if (frame.lights.size() >= kMaxLightsPerFrame) {
            ++stats.over_cap;
            continue;
        }
        const float strength = light_strength(light, time_seconds);
        const Vec2 top_left = camera.world_to_screen({box.x, box.y});
        SpriteDraw sprite;
        sprite.texture = texture;
        sprite.dest = {std::round(top_left.x), std::round(top_left.y), std::round(box.w),
                       std::round(box.h)};
        sprite.tint = {light.color.r, light.color.g, light.color.b,
                       static_cast<std::uint8_t>(std::lround(strength * 255.0f))};
        sprite.blend = BlendMode::Add;  // 빛은 더할수록 밝아져요
        frame.lights.push_back(sprite);
        ++stats.drawn;
    }
    return stats;
}

}  // namespace wind::engine
