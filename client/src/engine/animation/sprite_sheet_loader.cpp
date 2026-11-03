// 이 파일은 무엇을 하나요?
// 캐릭터 그림 설명서(JSON)를 한 칸씩 확인하는 "숙제 검사"를 실제로 하는 곳이에요.
// 숫자가 너무 크거나, 빠진 칸이 있거나, 모양이 틀리면 오류 목록에 적어요.

#include "engine/animation/sprite_sheet_loader.hpp"

#include "wind/content/content_path.hpp"

#include <limits>

namespace wind::engine {
namespace {

using content::JsonReader;

// 받아 줄 수 있는 값의 한계. 이보다 크면 실수라고 보고 거절해요.
constexpr std::int64_t kMaxFrameSize = 256;      // 그림 한 칸의 최대 크기(픽셀)
constexpr std::int64_t kMaxSheetIndex = 63;      // 행·열 번호 최대값
constexpr std::size_t kMaxClips = 32;            // 클립(동작) 최대 개수
constexpr std::size_t kMaxClipFrames = 64;       // 클립 하나의 최대 그림 수
constexpr std::int64_t kMaxFrameMs = 10'000;     // 그림 하나를 보여 주는 최대 시간(10초)

// clips 안에 있는 클립 하나("walk" 같은 것)를 읽어요.
std::optional<AnimationClip> parse_clip(JsonReader& reader) {
    AnimationClip clip;
    const auto* frames = reader.array("frames", kMaxClipFrames);
    const auto* frame_ms = reader.array("frame_ms", kMaxClipFrames);
    const auto loop = reader.boolean("loop");
    if (frames == nullptr || frame_ms == nullptr || !loop) {
        return std::nullopt;  // 꼭 필요한 칸이 없으면 더 볼 필요가 없어요
    }
    // 그림 번호 목록과 시간 목록은 개수가 같아야 짝이 맞아요.
    if (frames->empty() || frames->size() != frame_ms->size()) {
        reader.error(reader.field_path("frame_ms"),
                     "frames and frame_ms must be non-empty and the same length");
        return std::nullopt;
    }

    bool ok = true;
    for (std::size_t i = 0; i < frames->size(); ++i) {
        const auto& column = (*frames)[i];
        const auto& ms = (*frame_ms)[i];
        // 그림 번호는 0~63 사이의 정수여야 해요.
        if (!column.is_number_integer() || column.get<std::int64_t>() < 0 ||
            column.get<std::int64_t>() > kMaxSheetIndex) {
            reader.error(reader.index_path("frames", i),
                         "frame column must be an integer 0-" + std::to_string(kMaxSheetIndex));
            ok = false;
            continue;
        }
        // 0ms 그림은 영원히 다음 그림으로 못 넘어가는 문제를 만들 수 있어서 1ms 이상만 받아요.
        if (!ms.is_number_integer() || ms.get<std::int64_t>() < 1 ||
            ms.get<std::int64_t>() > kMaxFrameMs) {
            reader.error(reader.index_path("frame_ms", i),
                         "frame duration must be an integer 1-" + std::to_string(kMaxFrameMs));
            ok = false;
            continue;
        }
        clip.frames.push_back(static_cast<int>(column.get<std::int64_t>()));
        // 설명서는 밀리초(ms, 1000분의 1초)로 적지만, 게임 안에서는 초로 바꿔서 써요.
        clip.seconds.push_back(static_cast<double>(ms.get<std::int64_t>()) / 1000.0);
    }
    clip.loop = *loop;
    return ok ? std::optional<AnimationClip>(std::move(clip)) : std::nullopt;
}

}  // namespace

content::LoadResult<SpriteSheet> parse_sprite_sheet(const nlohmann::json& document,
                                                    const std::string& file) {
    content::ErrorList errors;  // 틀린 곳을 모아 두는 공책
    JsonReader reader(document, file, "", errors);
    SpriteSheet sheet;

    // 설명서 양식 버전이 우리가 아는 버전인지 확인해요.
    const auto version = reader.integer("schema_version", 1, std::numeric_limits<std::int32_t>::max());
    if (version && *version != kSpriteSheetSchemaVersion) {
        reader.error("schema_version", "unsupported schema version " + std::to_string(*version));
    }

    if (auto id = reader.string("id", 64)) sheet.id = std::move(*id);
    // 그림 파일 위치가 콘텐츠 폴더 밖을 가리키면 안 돼요 (안전 검사).
    if (auto image = reader.string("image")) {
        if (auto normalized = content::normalize_content_path(*image)) {
            sheet.image = std::move(*normalized);
        } else {
            reader.error("image", "path must stay inside the content folder: " + *image);
        }
    }
    const auto fw = reader.integer("frame_width", 1, kMaxFrameSize);
    const auto fh = reader.integer("frame_height", 1, kMaxFrameSize);
    if (fw) sheet.frame_width = static_cast<std::int32_t>(*fw);
    if (fh) sheet.frame_height = static_cast<std::int32_t>(*fh);

    // 발 위치는 그림 한 칸 안에 있어야 해요.
    if (const auto* origin = reader.object("origin"); origin != nullptr && fw && fh) {
        JsonReader origin_reader(*origin, file, "origin", errors);
        const auto x = origin_reader.integer("x", 0, *fw);
        const auto y = origin_reader.integer("y", 0, *fh);
        if (x && y) sheet.origin = {static_cast<std::int32_t>(*x), static_cast<std::int32_t>(*y)};
    }

    // 네 방향(아래·왼쪽·오른쪽·위) 모두 몇 번째 행인지 적혀 있어야 해요.
    if (const auto* rows = reader.object("direction_rows")) {
        JsonReader rows_reader(*rows, file, "direction_rows", errors);
        for (world::Direction dir : world::kAllDirections) {
            if (const auto row = rows_reader.integer(world::to_string(dir), 0, kMaxSheetIndex)) {
                sheet.direction_rows[static_cast<std::size_t>(dir)] = static_cast<std::int32_t>(*row);
            }
        }
    }

    // 클립(동작)들을 하나씩 읽어요.
    if (const auto* clips = reader.object("clips")) {
        if (clips->empty() || clips->size() > kMaxClips) {
            reader.error("clips", "must contain 1-" + std::to_string(kMaxClips) + " clips");
        }
        for (const auto& [name, value] : clips->items()) {
            JsonReader clip_reader(value, file, "clips." + name, errors);
            if (!clip_reader.valid()) {
                continue;
            }
            if (auto clip = parse_clip(clip_reader)) {
                sheet.clips.emplace(name, std::move(*clip));
            }
        }
    }

    // 틀린 곳이 하나라도 있으면 결과 없이 오류 목록만 돌려줘요.
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(sheet), {}};
}

content::LoadResult<SpriteSheet> load_sprite_sheet_file(const std::filesystem::path& content_root,
                                                        std::string_view path) {
    content::ErrorList errors;
    const std::string name(path);
    // 1) 파일 위치가 안전한지 확인
    const auto file = content::resolve_content_path(content_root, path);
    if (!file) {
        errors.add(name, "", "invalid content path");
        return {std::nullopt, errors.take()};
    }
    // 2) 파일을 읽어서 JSON으로 바꾸기
    const auto document = content::read_json_file(*file, name, errors);
    if (!document) {
        return {std::nullopt, errors.take()};
    }
    // 3) 내용 검사
    return parse_sprite_sheet(*document, name);
}

}  // namespace wind::engine
