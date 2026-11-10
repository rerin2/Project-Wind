// 이 파일은 무엇을 하나요?
// 타일셋 설명서를 한 칸씩 확인하며 읽는 곳이에요. (맵 읽기 파일에서 나눠 왔어요)
// 순서: 버전·이름·그림 위치·칸 크기 -> 카드 목록 -> 움직이는 카드의 그림 번호가 진짜 있는지

#include "wind/content/tileset_loader.hpp"

#include "wind/content/json_reader.hpp"

#include <limits>
#include <set>
#include <utility>

namespace wind::content {
namespace {

using world::TileId;

// 카드 번호로 쓸 수 있는 가장 큰 수, 타일셋 그림 한 줄의 최대 칸 수
constexpr std::int64_t kMaxTileId = std::numeric_limits<TileId>::max();
constexpr std::int64_t kMaxTilesetColumns = 256;
// 움직이는 카드: 한 장을 보여 주는 시간 범위 (초)
constexpr double kMinFrameSeconds = 0.05;
constexpr double kMaxFrameSeconds = 10.0;
// 흔들리는 카드: 최대 픽셀 수와 왕복 시간 범위 (초)
constexpr double kMaxSwayPixels = 4.0;
constexpr double kMinSwayPeriod = 0.5;
constexpr double kMaxSwayPeriod = 30.0;

// "animation": { "frames": [4, 14, 15], "frame_seconds": 0.3 }
// 그림 번호가 타일셋에 진짜 있는지는 카드를 다 읽은 다음에 확인해요.
std::optional<world::TileAnimation> parse_animation(JsonReader& tile_reader) {
    const auto* object = tile_reader.object("animation");
    if (object == nullptr) {
        return std::nullopt;
    }
    JsonReader reader(*object, tile_reader.file(), tile_reader.field_path("animation"),
                      tile_reader.errors());
    world::TileAnimation animation;
    const auto seconds = reader.number("frame_seconds", kMinFrameSeconds, kMaxFrameSeconds);
    const auto* frames = reader.array("frames", kMaxTileAnimationFrames);
    if (!seconds || frames == nullptr) {
        return std::nullopt;
    }
    if (frames->size() < 2) {
        reader.error(reader.field_path("frames"), "animation needs at least 2 frames");
        return std::nullopt;
    }
    animation.frame_seconds = static_cast<float>(*seconds);
    for (std::size_t i = 0; i < frames->size(); ++i) {
        const auto& value = (*frames)[i];
        if (!value.is_number_integer() || value.get<std::int64_t>() < 1 ||
            value.get<std::int64_t>() > kMaxTileId) {
            reader.error(reader.index_path("frames", i),
                         "frame must be a tile id 1-" + std::to_string(kMaxTileId));
            return std::nullopt;
        }
        animation.frames.push_back(static_cast<TileId>(value.get<std::int64_t>()));
    }
    return animation;
}

// "sway": { "pixels": 1, "period_seconds": 3 }
std::optional<world::TileSway> parse_sway(JsonReader& tile_reader) {
    const auto* object = tile_reader.object("sway");
    if (object == nullptr) {
        return std::nullopt;
    }
    JsonReader reader(*object, tile_reader.file(), tile_reader.field_path("sway"),
                      tile_reader.errors());
    const auto pixels = reader.number("pixels", 0.0, kMaxSwayPixels);
    const auto period = reader.number("period_seconds", kMinSwayPeriod, kMaxSwayPeriod);
    if (!pixels || !period) {
        return std::nullopt;
    }
    return world::TileSway{static_cast<float>(*pixels), static_cast<float>(*period)};
}

// 움직이는 카드가 보여 줄 그림 번호가 모두 이 타일셋에 있나요?
void check_animation_frames(const world::Tileset& tileset, const std::string& file,
                            ErrorList& errors) {
    for (std::size_t i = 0; i < tileset.tiles.size(); ++i) {
        const auto& tile = tileset.tiles[i];
        if (!tile.animation) {
            continue;
        }
        for (std::size_t f = 0; f < tile.animation->frames.size(); ++f) {
            const TileId frame = tile.animation->frames[f];
            if (tileset.find(frame) == nullptr) {
                errors.add(file,
                           "tiles[" + std::to_string(i) + "].animation.frames[" +
                               std::to_string(f) + "]",
                           "frame tile id " + std::to_string(frame) + " is not defined in this tileset");
            }
        }
    }
}

}  // namespace

// 타일셋 설명서 검사: 버전, 이름, 그림 위치, 칸 크기, 카드 목록(번호 중복 없이)
LoadResult<world::Tileset> parse_tileset(const nlohmann::json& document, const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    world::Tileset tileset;

    if (reader.valid() && reader.schema_version(kTilesetSchemaVersion)) {
        if (auto id = reader.string("id", 64)) tileset.id = std::move(*id);
        if (auto image = reader.content_path("image")) tileset.image = std::move(*image);
        if (auto w = reader.integer("tile_width", kMinTileSize, kMaxTileSize)) tileset.tile_width = static_cast<std::int32_t>(*w);
        if (auto h = reader.integer("tile_height", kMinTileSize, kMaxTileSize)) tileset.tile_height = static_cast<std::int32_t>(*h);
        if (auto c = reader.integer("columns", 1, kMaxTilesetColumns)) tileset.columns = static_cast<std::int32_t>(*c);

        if (const auto* tiles = reader.array("tiles", kMaxTilesetTiles)) {
            std::set<TileId> seen;  // 이미 나온 카드 번호 (중복 찾기용)
            for (std::size_t i = 0; i < tiles->size(); ++i) {
                JsonReader tile_reader((*tiles)[i], file, reader.index_path("tiles", i), errors);
                const auto id = tile_reader.integer("id", 1, kMaxTileId);
                auto name = tile_reader.string("name", 64);
                const auto walkable = tile_reader.boolean("walkable");
                if (!id || !name || !walkable) {
                    continue;
                }
                const auto tile_id = static_cast<TileId>(*id);
                if (!seen.insert(tile_id).second) {
                    tile_reader.error(tile_reader.field_path("id"),
                                      "duplicate tile id " + std::to_string(tile_id));
                }
                world::TileProperties props{tile_id, std::move(*name), *walkable, {}, {}};
                // 움직임·흔들림은 없어도 돼요. 있으면 칸마다 검사해요.
                if (tile_reader.has("animation")) props.animation = parse_animation(tile_reader);
                if (tile_reader.has("sway")) props.sway = parse_sway(tile_reader);
                tileset.tiles.push_back(std::move(props));
            }
        }
        check_animation_frames(tileset, file, errors);
    }

    // 오류가 하나라도 있으면 결과 대신 오류 목록을 돌려줘요.
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(tileset), {}};
}

}  // namespace wind::content
