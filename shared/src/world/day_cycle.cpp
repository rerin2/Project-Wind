// 이 파일은 무엇을 하나요?
// 하루 색 곡선에서 어떤 시각의 색을 찾아 앞뒤 점 사이를 부드럽게 섞는 곳이에요.

#include "wind/world/day_cycle.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace wind::world {
namespace {

constexpr float kHoursPerDay = 24.0f;

std::uint8_t mix(std::uint8_t a, std::uint8_t b, float t) {
    return static_cast<std::uint8_t>(
        std::lround(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t));
}

// 두 점 사이 t(0~1) 위치의 색
DaySample blend(const DayKey& a, const DayKey& b, float t) {
    return {{mix(a.ambient.r, b.ambient.r, t), mix(a.ambient.g, b.ambient.g, t),
             mix(a.ambient.b, b.ambient.b, t), mix(a.ambient.a, b.ambient.a, t)},
            a.night + (b.night - a.night) * t};
}

}  // namespace

DaySample sample_day_cycle(const DayCycle& cycle, float hour) {
    const auto& keys = cycle.keys;
    if (keys.empty()) {
        return {};
    }
    if (keys.size() == 1) {
        return {keys.front().ambient, keys.front().night};
    }
    // 시각을 0~24 안으로 감아요. (25시 = 1시, -1시 = 23시)
    float h = std::fmod(hour, kHoursPerDay);
    if (h < 0.0f) {
        h += kHoursPerDay;
    }
    // h 바로 뒤에 오는 점을 찾아요.
    std::size_t next = 0;
    while (next < keys.size() && keys[next].hour <= h) {
        ++next;
    }
    // 첫 점 앞이거나 마지막 점 뒤면, 마지막 점 -> (다음 날) 첫 점 사이예요.
    const DayKey& before = keys[(next + keys.size() - 1) % keys.size()];
    const DayKey& after = keys[next % keys.size()];
    float start = before.hour;
    float end = after.hour;
    float now = h;
    if (end <= start) {  // 자정을 넘는 구간
        end += kHoursPerDay;
        if (now < start) {
            now += kHoursPerDay;
        }
    }
    const float span = end - start;
    const float t = span > 0.0f ? (now - start) / span : 0.0f;
    return blend(before, after, t);
}

}  // namespace wind::world
