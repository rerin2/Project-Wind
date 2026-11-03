#pragma once

// 이 파일은 무엇을 하나요?
// 맵 설명서와 타일셋 설명서(JSON 파일)를 읽어서 게임이 쓸 수 있는 맵으로 만들어 줘요 (명세 6.1, 16.4).
// 읽으면서 숙제 검사를 꼼꼼히 해요. 하나라도 틀리면 맵을 주지 않고,
// "어느 파일의 어느 칸이 틀렸는지" 적힌 오류 목록을 돌려줘요.
// 망가진 맵으로 게임을 시작하는 것보다, 처음부터 알려 주는 게 훨씬 안전하니까요.

#include "wind/content/json_reader.hpp"
#include "wind/world/map.hpp"

#include <filesystem>
#include <string_view>

namespace wind::content {

// 지금 읽을 수 있는 설명서 양식의 버전.
// 나중에 양식이 바뀌면 숫자를 올려서, 옛 양식과 새 양식을 헷갈리지 않게 해요.
inline constexpr std::int64_t kMapSchemaVersion = 1;
inline constexpr std::int64_t kTilesetSchemaVersion = 1;

// 맵 크기·구성의 최대값. 이보다 크면 실수이거나 나쁜 파일일 수 있어서 거절해요.
inline constexpr std::int64_t kMaxMapSide = 512;          // 가로·세로 최대 칸 수
inline constexpr std::size_t kMaxMapLayers = 8;           // 겹치는 필름 최대 장 수
inline constexpr std::size_t kMaxSpawnPoints = 64;        // 등장 자리 최대 개수
inline constexpr std::int64_t kMinTileSize = 8;           // 칸 한 변의 최소 픽셀
inline constexpr std::int64_t kMaxTileSize = 128;         // 칸 한 변의 최대 픽셀
inline constexpr std::size_t kMaxTilesetTiles = 4096;     // 타일셋 안 카드 최대 종류

// 맵과, 그 맵이 쓰는 타일셋을 한 상자에 담아요.
struct LoadedMap {
    world::MapData map;
    world::Tileset tileset;
};

// 이미 JSON으로 바꾼 타일셋 설명서를 검사해서 Tileset으로 만들어요.
[[nodiscard]] LoadResult<world::Tileset> parse_tileset(const nlohmann::json& document,
                                                       const std::string& file);

// 이미 JSON으로 바꾼 맵 설명서를 검사해서 MapData로 만들어요.
// 맵에 쓰인 카드 번호가 타일셋에 진짜 있는지 확인하고,
// 칸마다 지나갈 수 있는지(walkable) 미리 계산해 둬요.
[[nodiscard]] LoadResult<world::MapData> parse_map(const nlohmann::json& document,
                                                   const std::string& file,
                                                   const world::Tileset& tileset);

// 콘텐츠 폴더에서 맵 파일을 읽고, 맵에 적힌 타일셋 파일도 찾아 읽어서 함께 검사해요.
// map_path는 콘텐츠 폴더 기준 위치예요 (예: "maps/test_meadow.json").
[[nodiscard]] LoadResult<LoadedMap> load_map_file(const std::filesystem::path& content_root,
                                                  std::string_view map_path);

}  // namespace wind::content
