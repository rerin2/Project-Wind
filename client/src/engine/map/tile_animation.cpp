// 이 파일은 무엇을 하나요?
// 움직이는 타일의 "지금 카드"와 흔들리는 타일의 "지금 밀린 픽셀"을 계산해요.

#include "engine/map/tile_animation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numbers>

namespace wind::engine {
namespace {

// 칸마다 박자를 얼마나 어긋나게 할지 정하는 수 (칸 위치로 섞어요. 서로 다른 소수를 써서 줄무늬가 안 생겨요)
constexpr std::uint32_t kStaggerX = 7;
constexpr std::uint32_t kStaggerY = 13;
// 바람 물결: 오른쪽 칸일수록, 아래 칸일수록 조금 늦게 바람을 맞아요 (라디안)
constexpr float kWaveX = 0.45f;
constexpr float kWaveY = 0.2f;

}  // namespace

world::TileId animated_tile(const world::TileProperties& tile, float time_seconds,
                            world::TileCoord cell) {
    if (!tile.animation || tile.animation->frames.empty() || tile.animation->frame_seconds <= 0.0f) {
        return tile.id;
    }
    const auto& frames = tile.animation->frames;
    // 지금까지 몇 장 넘겼는지 + 칸마다 다른 출발 장
    const auto ticks = static_cast<std::uint64_t>(std::max(0.0f, time_seconds) /
                                                  tile.animation->frame_seconds);
    const auto stagger = static_cast<std::uint32_t>(cell.x) * kStaggerX +
                         static_cast<std::uint32_t>(cell.y) * kStaggerY;
    return frames[(ticks + stagger) % frames.size()];
}

float sway_offset(const world::TileSway& sway, float time_seconds, world::TileCoord cell) {
    if (sway.pixels <= 0.0f || sway.period_seconds <= 0.0f) {
        return 0.0f;
    }
    // 사인(sin) 곡선: -1 ~ 1 사이를 부드럽게 왔다 갔다 해요. 칸 위치만큼 늦게 출발해요.
    const float angle = 2.0f * std::numbers::pi_v<float> * time_seconds / sway.period_seconds -
                        static_cast<float>(cell.x) * kWaveX - static_cast<float>(cell.y) * kWaveY;
    return std::round(std::sin(angle) * sway.pixels);
}

}  // namespace wind::engine
