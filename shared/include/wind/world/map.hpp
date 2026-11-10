#pragma once

// 이 파일은 무엇을 하나요?
// 게임 맵과 타일셋(타일 그림 모음)을 컴퓨터 기억 속에 어떻게 담을지 정해요.
// 맵 = 커다란 바둑판, 타일 = 바둑판 한 칸에 놓는 그림 카드라고 생각하면 돼요.
// 그림을 그리는 일과는 상관없는 "순수한 정보"라서 도구(맵 검사기 등)도 그대로 써요.
// 설명서(JSON) 모양과 검사 규칙은 docs/ENGINE.md, 읽어 오는 코드는 wind/content/map_loader.hpp에 있어요.

#include "wind/world/ambience.hpp"
#include "wind/world/direction.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::world {

// 타일 번호. 0은 "이 칸에는 카드가 없어요"라는 뜻이에요.
// 1번부터는 타일셋 그림의 칸 순서와 짝이 맞아요 (1번 = 그림의 왼쪽 맨 위 칸).
using TileId = std::uint16_t;
inline constexpr TileId kEmptyTile = 0;

// 투명 필름을 여러 장 겹친다고 생각해 보세요. 이것은 "몇 번째 필름에 그리느냐"예요 (명세 5.5).
// 바닥 -> 바닥 장식 -> 낮은 물건 -> 높은 물건 -> 맨 앞 순서로 쌓여요.
// 캐릭터는 낮은 물건(LowObjects)과 높은 물건(HighObjects) 사이에 그려져요.
enum class RenderLayer : std::uint8_t { Ground, GroundDecor, LowObjects, HighObjects, Foreground };

// 움직이는 타일 (명세 7.1): 그림 카드 몇 장을 차례로 바꿔 보여 줘요. (물 반짝임, 꽃 흔들림)
// 보여 주기 전용이라 지나갈 수 있는지는 맵에 적힌 원래 카드로 정해요.
struct TileAnimation {
    std::vector<TileId> frames;   // 차례로 보여 줄 카드 번호들 (모두 같은 타일셋에 있어야 해요)
    float frame_seconds = 0.25f;  // 카드 한 장을 보여 주는 시간 (초)
};

// 바람에 흔들리는 타일 (풀, 나뭇잎): 좌우로 몇 픽셀씩 살랑살랑 움직여요.
struct TileSway {
    float pixels = 1.0f;          // 가장 많이 움직일 때 몇 픽셀
    float period_seconds = 3.0f;  // 한 번 왔다 갔다 하는 데 걸리는 시간 (초)
};

// 타일 카드 한 종류의 성질 (예: "풀", 지나갈 수 있음)
struct TileProperties {
    TileId id = kEmptyTile;
    std::string name;
    bool walkable = true;  // false면 이 카드가 놓인 칸으로는 걸어갈 수 없어요 (벽, 물 등)
    std::optional<TileAnimation> animation;  // 있으면 카드를 바꿔 가며 보여 줘요
    std::optional<TileSway> sway;            // 있으면 바람에 흔들려요
};

// 타일셋: 타일 그림 한 장과, 그 안에 있는 카드 하나하나의 성질 목록
struct Tileset {
    std::string id;
    std::string image;  // 그림 파일 위치. 콘텐츠 폴더(게임 자료 폴더)를 기준으로 적어요
    std::int32_t tile_width = 0;
    std::int32_t tile_height = 0;
    std::int32_t columns = 0;  // 그림 한 줄에 카드가 몇 장 있는지
    // vector(벡터: 늘었다 줄었다 하는 상자 줄)에 카드 성질을 차례로 담아요.
    std::vector<TileProperties> tiles;

    // 번호로 카드 성질을 찾아요. 그런 번호가 없으면 nullptr(빈손)을 돌려줘요.
    [[nodiscard]] const TileProperties* find(TileId tile) const;
};

// 필름 한 장: 맵의 모든 칸에 어떤 카드가 놓였는지 적은 목록
struct MapLayer {
    std::string name;
    RenderLayer render_layer = RenderLayer::Ground;
    // 칸 번호를 왼쪽 위부터 오른쪽으로 한 줄씩 읽어 가며 적어요 (row-major: 줄 우선).
    // 그래서 (x, y) 칸의 자리 번호는 y * 가로칸수 + x 예요.
    std::vector<TileId> tiles;
};

// 캐릭터가 맵에 처음 나타나는 자리
struct SpawnPoint {
    std::string id;
    TileCoord position;
};

// 맵을 옮길 때 화면을 가리는 방법 (명세 6.8)
// Fade: 화면 전체가 서서히 까매져요. Iris: 주인공을 가운데 두고 둥근 구멍이 오므라들어요.
enum class ScreenTransition : std::uint8_t { Fade, Iris };

// 다른 맵으로 가는 출구 (명세 7.4)
// 캐릭터의 발이 area 칸들 중 하나에 들어가면 to_map 맵의 to_spawn 자리로 옮겨 가요.
struct MapExit {
    TileCoord area_min;  // 출구 영역의 왼쪽 위 칸
    TileCoord area_max;  // 출구 영역의 오른쪽 아래 칸 (포함)
    std::string to_map;    // 갈 맵 파일 위치 (콘텐츠 폴더 기준)
    std::string to_spawn;  // 그 맵의 어느 등장 자리로 갈지
    ScreenTransition transition = ScreenTransition::Fade;  // 화면 가리는 방법 (없으면 페이드)

    [[nodiscard]] bool contains(TileCoord tile) const {
        return tile.x >= area_min.x && tile.x <= area_max.x && tile.y >= area_min.y &&
               tile.y <= area_max.y;
    }
};

// 맵 한 장의 모든 정보
struct MapData {
    std::string map_id;
    std::string display_name;
    std::string tileset_path;  // 이 맵이 쓰는 타일셋 설명서 위치 (콘텐츠 폴더 기준)
    std::int32_t tile_size = 0;
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::vector<MapLayer> layers;
    std::vector<SpawnPoint> spawn_points;
    std::vector<MapExit> exits;  // 다른 맵으로 가는 출구들 (없어도 돼요)
    // 맵 분위기: 늘 떠다니는 효과, 주변광, 빛들. 없어도 돼요.
    Ambience ambience;
    // 칸마다 "지나갈 수 있나?"를 미리 계산해 둔 표예요. 1이면 갈 수 있고 0이면 못 가요.
    // 맵을 읽을 때 로더가 카드 성질을 보고 채워 줘요.
    std::vector<std::uint8_t> walkable;

    // 이 칸이 바둑판 안에 있나요?
    [[nodiscard]] bool in_bounds(TileCoord pos) const;
    // 이 칸으로 걸어갈 수 있나요? 바둑판 밖이면 언제나 "못 가요"(false)
    [[nodiscard]] bool is_walkable(TileCoord pos) const;
    // 이 필름의 이 칸에 놓인 카드 번호. 바둑판 밖이면 "카드 없음"(kEmptyTile)
    [[nodiscard]] TileId tile_at(const MapLayer& layer, TileCoord pos) const;
    // 이 칸에 있는 출구를 찾아요. 없으면 nullptr
    [[nodiscard]] const MapExit* exit_at(TileCoord tile) const;
    // 이름으로 등장 자리를 찾아요. 없으면 nullptr
    [[nodiscard]] const SpawnPoint* find_spawn(std::string_view spawn_id) const;
    // (x, y) 칸을 목록의 자리 번호로 바꿔요. 바둑판 안인지는 먼저 확인하고 불러야 해요.
    [[nodiscard]] std::size_t index_of(TileCoord pos) const;
};

}  // namespace wind::world
