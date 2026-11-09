// 이 파일은 무엇을 하나요?
// 맵 설명서와 타일셋 설명서를 숙제 검사하듯 한 칸씩 확인하면서 읽는 곳이에요.
// 순서: 타일셋을 먼저 읽고 -> 맵을 읽으면서 "맵에 쓴 카드가 타일셋에 진짜 있나?" 확인하고
//       -> 칸마다 지나갈 수 있는지 계산하고 -> 등장 자리가 막힌 칸이 아닌지 확인해요.
// 틀린 곳이 하나라도 있으면 결과 대신 오류 목록을 돌려줘요.

#include "wind/content/map_loader.hpp"

#include "wind/content/ambience_loader.hpp"
#include "wind/content/content_path.hpp"
#include "wind/content/exit_loader.hpp"

#include <array>
#include <limits>
#include <set>
#include <utility>

namespace wind::content {
namespace {

using world::RenderLayer;
using world::TileId;

// 카드 번호로 쓸 수 있는 가장 큰 수, 타일셋 그림 한 줄의 최대 칸 수
constexpr std::int64_t kMaxTileId = std::numeric_limits<TileId>::max();
constexpr std::int64_t kMaxTilesetColumns = 256;

// 설명서에 적는 필름 이름(글자)과 실제 필름 종류의 짝 목록
constexpr std::array<std::pair<std::string_view, RenderLayer>, 5> kRenderLayerNames{{
    {"ground", RenderLayer::Ground},
    {"ground_decor", RenderLayer::GroundDecor},
    {"low_objects", RenderLayer::LowObjects},
    {"high_objects", RenderLayer::HighObjects},
    {"foreground", RenderLayer::Foreground},
}};

// 필름 이름 글자를 보고 필름 종류를 찾아요. 모르는 이름이면 "없음"
std::optional<RenderLayer> parse_render_layer(std::string_view text) {
    for (const auto& [name, layer] : kRenderLayerNames) {
        if (name == text) {
            return layer;
        }
    }
    return std::nullopt;
}

// 설명서 양식 버전(schema_version)이 우리가 아는 버전인지 확인해요.
// 모르는 버전이면 칸 뜻이 다를 수 있어서 읽지 않아요.
bool check_schema(JsonReader& reader, std::int64_t expected) {
    const auto version = reader.integer("schema_version", 1, std::numeric_limits<std::int32_t>::max());
    if (version && *version != expected) {
        reader.error(reader.field_path("schema_version"),
                     "unsupported schema version " + std::to_string(*version) + " (expected " +
                         std::to_string(expected) + ")");
        return false;
    }
    return version.has_value();
}

// 파일 위치 칸을 읽고, 콘텐츠 폴더(상자) 밖을 가리키지 않는지 문지기 검사를 해요.
std::optional<std::string> read_content_path(JsonReader& reader, std::string_view key) {
    const auto raw = reader.string(key);
    if (!raw) {
        return std::nullopt;
    }
    auto normalized = normalize_content_path(*raw);
    if (!normalized) {
        reader.error(reader.field_path(key), "path must stay inside the content folder: " + *raw);
    }
    return normalized;
}

// 출구 영역 안에 걸어 들어갈 수 있는 칸이 하나라도 있나요?
bool exit_is_reachable(const world::MapData& map, const world::MapExit& exit) {
    for (std::int32_t y = exit.area_min.y; y <= exit.area_max.y; ++y) {
        for (std::int32_t x = exit.area_min.x; x <= exit.area_max.x; ++x) {
            if (map.is_walkable({x, y})) {
                return true;
            }
        }
    }
    return false;
}

// 칸마다 "지나갈 수 있나?"를 계산해요.
// 규칙: 카드가 하나라도 놓여 있고, 놓인 카드가 전부 "지나갈 수 있음"이면 OK.
// 카드가 하나도 없는 칸은 허공이라 못 가요. 카드 하나라도 벽이면 못 가요.
void compute_walkability(world::MapData& map, const world::Tileset& tileset) {
    const auto cells = static_cast<std::size_t>(map.width) * static_cast<std::size_t>(map.height);
    map.walkable.assign(cells, 0);  // 처음엔 모두 "못 가요"로 채워 두고 하나씩 확인
    for (std::size_t i = 0; i < cells; ++i) {
        bool any_tile = false;
        bool blocked = false;
        // 같은 칸에 겹쳐 있는 필름들을 모두 살펴봐요.
        for (const auto& layer : map.layers) {
            const TileId tile = layer.tiles[i];
            if (tile == world::kEmptyTile) {
                continue;
            }
            any_tile = true;
            const auto* props = tileset.find(tile);
            blocked = blocked || props == nullptr || !props->walkable;
        }
        map.walkable[i] = (any_tile && !blocked) ? 1 : 0;
    }
}

// 맵의 필름(레이어)들을 읽어요.
// 필름마다 이름이 겹치지 않는지, 필름 종류가 맞는지, 칸 수가 가로x세로와 딱 맞는지,
// 카드 번호가 타일셋에 있는지 하나씩 확인해요.
void parse_layers(JsonReader& reader, world::MapData& map, const world::Tileset& tileset) {
    const auto* layers = reader.array("layers", kMaxMapLayers);
    if (layers == nullptr) {
        return;
    }
    if (layers->empty()) {
        reader.error(reader.field_path("layers"), "map needs at least one layer");
        return;
    }

    const auto cell_count = static_cast<std::size_t>(map.width) * static_cast<std::size_t>(map.height);
    // set(셋: 같은 것이 두 번 들어가지 않는 주머니)으로 이름 중복을 잡아요.
    std::set<std::string> names;

    for (std::size_t i = 0; i < layers->size(); ++i) {
        JsonReader layer_reader((*layers)[i], reader.file(), reader.index_path("layers", i),
                                reader.errors());
        if (!layer_reader.valid()) {
            continue;
        }

        world::MapLayer layer;
        if (auto name = layer_reader.string("name", 64)) {
            if (!names.insert(*name).second) {
                layer_reader.error(layer_reader.field_path("name"), "duplicate layer name: " + *name);
            }
            layer.name = std::move(*name);
        }
        if (auto layer_name = layer_reader.string("render_layer", 32)) {
            if (auto parsed = parse_render_layer(*layer_name)) {
                layer.render_layer = *parsed;
            } else {
                layer_reader.error(layer_reader.field_path("render_layer"),
                                   "unknown render layer: " + *layer_name);
            }
        }

        // 칸 목록은 정확히 가로x세로 개여야 해요. 하나라도 모자라거나 남으면 그림이 밀려요.
        const auto* tiles = layer_reader.array("tiles", cell_count);
        if (tiles == nullptr) {
            continue;
        }
        if (tiles->size() != cell_count) {
            layer_reader.error(layer_reader.field_path("tiles"),
                               "expected width*height = " + std::to_string(cell_count) +
                                   " tiles, found " + std::to_string(tiles->size()));
            continue;
        }

        layer.tiles.reserve(cell_count);
        for (std::size_t t = 0; t < tiles->size(); ++t) {
            const auto& value = (*tiles)[t];
            const auto field = layer_reader.index_path("tiles", t);
            // 카드 번호는 0 이상의 정수여야 해요. 틀리면 적어 두고 일단 빈 칸으로 채워요.
            if (!value.is_number_integer() || value.get<std::int64_t>() < 0 ||
                value.get<std::int64_t>() > kMaxTileId) {
                layer_reader.error(field, "tile id must be an integer 0-" + std::to_string(kMaxTileId));
                layer.tiles.push_back(world::kEmptyTile);
                continue;
            }
            const auto tile = static_cast<TileId>(value.get<std::int64_t>());
            // 타일셋에 없는 카드 번호는 그릴 그림이 없으니 틀린 것이에요.
            if (tile != world::kEmptyTile && tileset.find(tile) == nullptr) {
                layer_reader.error(field, "tile id " + std::to_string(tile) +
                                              " is not defined in tileset " + tileset.id);
            }
            layer.tiles.push_back(tile);
        }
        map.layers.push_back(std::move(layer));
    }
}

// 등장 자리들을 읽어요. 위치가 바둑판 안인지, 이름이 겹치지 않는지 확인해요.
void parse_spawn_points(JsonReader& reader, world::MapData& map) {
    const auto* spawns = reader.array("spawn_points", kMaxSpawnPoints);
    if (spawns == nullptr) {
        return;
    }
    if (spawns->empty()) {
        reader.error(reader.field_path("spawn_points"), "map needs at least one spawn point");
        return;
    }

    std::set<std::string> ids;
    for (std::size_t i = 0; i < spawns->size(); ++i) {
        JsonReader spawn_reader((*spawns)[i], reader.file(), reader.index_path("spawn_points", i),
                                reader.errors());
        if (!spawn_reader.valid()) {
            continue;
        }
        auto id = spawn_reader.string("id", 64);
        // x는 0 ~ (가로칸수-1), y는 0 ~ (세로칸수-1) 사이여야 바둑판 안이에요.
        const auto x = spawn_reader.integer("x", 0, map.width - 1);
        const auto y = spawn_reader.integer("y", 0, map.height - 1);
        if (!id || !x || !y) {
            continue;
        }
        if (!ids.insert(*id).second) {
            spawn_reader.error(spawn_reader.field_path("id"), "duplicate spawn id: " + *id);
        }
        map.spawn_points.push_back(
            {std::move(*id), {static_cast<std::int32_t>(*x), static_cast<std::int32_t>(*y)}});
    }
}

}  // namespace

// 타일셋 설명서 검사: 버전, 이름, 그림 위치, 칸 크기, 카드 목록(번호 중복 없이)
LoadResult<world::Tileset> parse_tileset(const nlohmann::json& document, const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    world::Tileset tileset;

    if (reader.valid() && check_schema(reader, kTilesetSchemaVersion)) {
        if (auto id = reader.string("id", 64)) tileset.id = std::move(*id);
        if (auto image = read_content_path(reader, "image")) tileset.image = std::move(*image);
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
                tileset.tiles.push_back({tile_id, std::move(*name), *walkable});
            }
        }
    }

    // 오류가 하나라도 있으면 결과 대신 오류 목록을 돌려줘요.
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(tileset), {}};
}

// 맵 설명서 검사: 버전, 이름, 타일셋 위치, 칸 크기, 가로·세로, 필름들, 등장 자리
LoadResult<world::MapData> parse_map(const nlohmann::json& document, const std::string& file,
                                     const world::Tileset& tileset) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    world::MapData map;

    if (reader.valid() && check_schema(reader, kMapSchemaVersion)) {
        if (auto id = reader.string("map_id", 64)) map.map_id = std::move(*id);
        if (auto name = reader.string("display_name", 128)) map.display_name = std::move(*name);
        if (auto path = read_content_path(reader, "tileset")) map.tileset_path = std::move(*path);

        const auto tile_size = reader.integer("tile_size", kMinTileSize, kMaxTileSize);
        const auto width = reader.integer("width", 1, kMaxMapSide);
        const auto height = reader.integer("height", 1, kMaxMapSide);

        if (tile_size) {
            map.tile_size = static_cast<std::int32_t>(*tile_size);
            // 지금은 맵 칸 크기와 타일셋 그림 칸 크기가 같아야 해요 (그래야 그림이 칸에 딱 맞아요).
            if (map.tile_size != tileset.tile_width || map.tile_size != tileset.tile_height) {
                reader.error(reader.field_path("tile_size"),
                             "must match tileset tile size " + std::to_string(tileset.tile_width) +
                                 "x" + std::to_string(tileset.tile_height));
            }
        }

        // 가로·세로를 알아야 필름 칸 수와 등장 자리 위치를 검사할 수 있어요.
        if (width && height) {
            map.width = static_cast<std::int32_t>(*width);
            map.height = static_cast<std::int32_t>(*height);
            parse_layers(reader, map, tileset);
            parse_spawn_points(reader, map);
        }

        // 분위기(ambience: 주변 효과, 조명)는 없어도 돼요. 있으면 칸마다 검사해요.
        if (reader.has("ambience")) {
            if (const auto* ambience = reader.object("ambience")) {
                parse_ambience(*ambience, file, static_cast<float>(map.width),
                               static_cast<float>(map.height), errors, map.ambience);
            }
        }

        // 다른 맵으로 가는 출구(exits)는 없어도 돼요. 있으면 영역과 갈 곳을 검사해요.
        if (reader.has("exits") && width && height) {
            if (const auto* exits = reader.array("exits", kMaxMapExits)) {
                parse_exits(*exits, file, map, errors);
            }
        }

        // 여기까지 틀린 곳이 없을 때만 "지나갈 수 있는 칸" 표를 만들어요.
        if (errors.empty()) {
            compute_walkability(map, tileset);
            // 등장 자리가 벽 위라면 캐릭터가 갇혀 버려요. 그래서 꼭 지나갈 수 있는 칸이어야 해요.
            for (std::size_t i = 0; i < map.spawn_points.size(); ++i) {
                if (!map.is_walkable(map.spawn_points[i].position)) {
                    reader.error(reader.index_path("spawn_points", i),
                                 "spawn point is on a blocked tile");
                }
            }
            // 출구 영역에 걸어 들어갈 수 있는 칸이 하나도 없으면, 그 출구는 절대 못 써요.
            for (std::size_t i = 0; i < map.exits.size(); ++i) {
                if (!exit_is_reachable(map, map.exits[i])) {
                    reader.error(reader.index_path("exits", i), "exit area has no walkable tile");
                }
            }
        }
    }

    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(map), {}};
}

// 파일에서 맵을 읽는 전체 과정: 맵 파일 열기 -> 타일셋 위치 알아내기 -> 타일셋 읽기 -> 맵 검사
LoadResult<LoadedMap> load_map_file(const std::filesystem::path& content_root,
                                    std::string_view map_path) {
    ErrorList errors;
    const std::string map_name(map_path);

    // 맵 파일 위치부터 문지기 검사
    const auto map_file = resolve_content_path(content_root, map_path);
    if (!map_file) {
        errors.add(map_name, "", "invalid content path");
        return {std::nullopt, errors.take()};
    }
    const auto map_json = read_json_file(*map_file, map_name, errors);
    if (!map_json) {
        return {std::nullopt, errors.take()};
    }

    // 맵이 가리키는 타일셋을 먼저 읽어요. 맵을 검사하려면 카드 성질이 필요하거든요.
    JsonReader reader(*map_json, map_name, "", errors);
    const auto tileset_path = read_content_path(reader, "tileset");
    if (!tileset_path) {
        return {std::nullopt, errors.take()};
    }
    const auto tileset_json =
        read_json_file(*resolve_content_path(content_root, *tileset_path), *tileset_path, errors);
    if (!tileset_json) {
        return {std::nullopt, errors.take()};
    }

    auto tileset = parse_tileset(*tileset_json, *tileset_path);
    if (!tileset.ok()) {
        return {std::nullopt, std::move(tileset.errors)};
    }
    auto map = parse_map(*map_json, map_name, *tileset.value);
    if (!map.ok()) {
        return {std::nullopt, std::move(map.errors)};
    }
    // 둘 다 통과! 맵과 타일셋을 한 상자에 담아 돌려줘요.
    return {LoadedMap{std::move(*map.value), std::move(*tileset.value)}, {}};
}

}  // namespace wind::content
