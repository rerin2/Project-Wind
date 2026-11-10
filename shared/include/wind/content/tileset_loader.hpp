#pragma once

// 이 파일은 무엇을 하나요?
// 타일셋 설명서(JSON)를 숙제 검사하듯 확인하면서 읽어요. (명세 6.1, 7.1)
// 카드마다 번호·이름·지나갈 수 있는지를 읽고,
// 움직이는 카드(animation)와 바람에 흔들리는 카드(sway)도 읽어요.
// 하나라도 틀리면 타일셋 대신 "어디가 틀렸는지" 적힌 오류 목록을 돌려줘요.

#include "wind/content/json_file.hpp"
#include "wind/content/load_result.hpp"
#include "wind/world/map.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace wind::content {

// 지금 읽을 수 있는 타일셋 설명서 양식의 버전
inline constexpr std::int64_t kTilesetSchemaVersion = 1;
// 타일셋 안 카드 최대 종류
inline constexpr std::size_t kMaxTilesetTiles = 4096;
// 움직이는 카드 하나에 들어갈 수 있는 그림 장 수 최대값
inline constexpr std::size_t kMaxTileAnimationFrames = 16;
// 칸 한 변의 최소·최대 픽셀 (맵 칸 크기도 같은 범위를 써요)
inline constexpr std::int64_t kMinTileSize = 8;
inline constexpr std::int64_t kMaxTileSize = 128;

// 이미 JSON으로 바꾼 타일셋 설명서를 검사해서 Tileset으로 만들어요.
[[nodiscard]] LoadResult<world::Tileset> parse_tileset(const nlohmann::json& document,
                                                       const std::string& file);

}  // namespace wind::content
