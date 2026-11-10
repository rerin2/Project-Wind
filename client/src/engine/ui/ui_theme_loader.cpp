// 이 파일은 무엇을 하나요?
// UI 테마 파일을 한 칸씩 확인하며 읽는 곳이에요.

#include "engine/ui/ui_theme_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/error_list.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <utility>

namespace wind::engine {
namespace {

using content::JsonReader;

// 그림 판 크기 상한 (이상한 숫자 막기)
constexpr std::int64_t kMaxImageSide = 1024;
// 글꼴 크기 범위 (픽셀)
constexpr double kMinFontSize = 6.0;
constexpr double kMaxFontSize = 64.0;

// "panel": { "source": [x, y, w, h], "border": 8 }
void read_frame(JsonReader& reader, std::string_view key, UiFrame& out) {
    const auto* object = reader.object(key);
    if (object == nullptr) {
        return;
    }
    JsonReader frame(*object, reader.file(), reader.field_path(key), reader.errors());
    const auto* source = frame.array("source", 4);
    const auto border = frame.integer("border", 0, kMaxImageSide / 2);
    if (source == nullptr || !border) {
        return;
    }
    std::int64_t numbers[4] = {};
    if (source->size() != 4) {
        frame.error(frame.field_path("source"), "expected [x, y, w, h]");
        return;
    }
    for (std::size_t i = 0; i < 4; ++i) {
        const auto& value = (*source)[i];
        const std::int64_t min = i < 2 ? 0 : 1;
        if (!value.is_number_integer() || value.get<std::int64_t>() < min ||
            value.get<std::int64_t>() > kMaxImageSide) {
            frame.error(frame.index_path("source", i),
                        "must be an integer " + std::to_string(min) + "-" + std::to_string(kMaxImageSide));
            return;
        }
        numbers[i] = value.get<std::int64_t>();
    }
    // 테두리가 그림의 절반보다 두꺼우면 가운데 조각이 없어져요.
    if (*border * 2 > numbers[2] || *border * 2 > numbers[3]) {
        frame.error(frame.field_path("border"), "border must be at most half the frame size");
        return;
    }
    out.source = {static_cast<std::int32_t>(numbers[0]), static_cast<std::int32_t>(numbers[1]),
                  static_cast<std::int32_t>(numbers[2]), static_cast<std::int32_t>(numbers[3])};
    out.border = static_cast<std::int32_t>(*border);
}

// "regular": { "file": "fonts/Galmuri11.ttf", "size": 12 }
void read_font(JsonReader& reader, std::string_view key, UiFont& out) {
    const auto* object = reader.object(key);
    if (object == nullptr) {
        return;
    }
    JsonReader font(*object, reader.file(), reader.field_path(key), reader.errors());
    if (auto file = font.content_path("file")) out.file = std::move(*file);
    if (auto size = font.number("size", kMinFontSize, kMaxFontSize)) out.pixel_size = static_cast<float>(*size);
}

// 색 칸 읽기 (없으면 기본 색 그대로)
void read_color(JsonReader& reader, std::string_view key, Color& out) {
    if (!reader.has(key)) {
        return;
    }
    if (const auto color = reader.color(key)) {
        out = {color->r, color->g, color->b, color->a};
    }
}

}  // namespace

content::LoadResult<UiTheme> parse_ui_theme(const nlohmann::json& document, const std::string& file) {
    content::ErrorList errors;
    JsonReader reader(document, file, "", errors);
    UiTheme theme;
    if (reader.valid() && reader.schema_version(kUiThemeSchemaVersion)) {
        if (auto image = reader.content_path("image")) theme.image = std::move(*image);
        if (const auto* fonts = reader.object("fonts")) {
            JsonReader font_reader(*fonts, file, "fonts", errors);
            read_font(font_reader, "regular", theme.regular);
            read_font(font_reader, "bold", theme.bold);
        }
        read_frame(reader, "panel", theme.panel);
        read_frame(reader, "button", theme.button);
        read_frame(reader, "button_focused", theme.button_focused);
        if (reader.has("colors")) {
            if (const auto* colors = reader.object("colors")) {
                JsonReader color_reader(*colors, file, "colors", errors);
                read_color(color_reader, "text", theme.text);
                read_color(color_reader, "text_dim", theme.text_dim);
                read_color(color_reader, "text_focused", theme.text_focused);
                read_color(color_reader, "text_shadow", theme.text_shadow);
            }
        }
        if (reader.has("panel_alpha")) {
            if (const auto alpha = reader.integer("panel_alpha", 0, 255)) {
                theme.panel_alpha = static_cast<std::uint8_t>(*alpha);
            }
        }
    }
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(theme), {}};
}

content::LoadResult<UiTheme> load_ui_theme_file(const std::filesystem::path& content_root,
                                                std::string_view path) {
    content::ErrorList errors;
    const std::string name(path);
    const auto file = content::resolve_content_path(content_root, path);
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = content::read_json_file(*file, name, errors);
    if (!document) {
        return {std::nullopt, errors.take()};
    }
    return parse_ui_theme(*document, name);
}

}  // namespace wind::engine
