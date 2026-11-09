#pragma once

// 이 파일은 무엇을 하나요?
// 스프라이트 시트 = 캐릭터 그림을 표처럼 모아 둔 한 장의 그림이에요.
// 가로(열)는 동작 순서, 세로(행)는 방향이에요. 예: 1행 = 아래를 보는 모습
// 어떤 칸을 오려 쓸지 계산해 줘요.

#include "engine/animation/animation_clip.hpp"
#include "engine/core/math.hpp"

#include "wind/world/direction.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace wind::engine {

struct SpriteSheet {
    std::string id;
    std::string image;  // 그림 파일 위치 (콘텐츠 폴더 기준)
    std::int32_t frame_width = 0;   // 그림 한 칸의 가로 크기
    std::int32_t frame_height = 0;  // 그림 한 칸의 세로 크기
    // 그림 한 칸 안에서 "발"이 있는 점. 캐릭터의 맵 위치에 이 점을 맞춰서 그려요.
    Vec2i origin;
    std::array<std::int32_t, 4> direction_rows{};  // 방향(아래·왼쪽·오른쪽·위)마다 몇 번째 행인지
    std::map<std::string, AnimationClip, std::less<>> clips;  // 이름 -> 클립 ("idle", "walk" ...)

    // 이름으로 클립을 찾아요. 없으면 nullptr(빈손)
    [[nodiscard]] const AnimationClip* clip(std::string_view name) const;
    // 방향과 칸 번호로, 그림 판에서 오려 낼 네모를 알려 줘요.
    [[nodiscard]] RectI frame_rect(world::Direction dir, int column) const;
};

}  // namespace wind::engine
