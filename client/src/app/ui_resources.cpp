// 이 파일은 무엇을 하나요?
// UI 테마를 읽고, 테마에 적힌 글꼴 두 개와 테두리 그림을 불러와요.

#include "app/ui_resources.hpp"

#include "engine/ui/ui_theme_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/error_list.hpp"
#include "wind/log.hpp"

#include <string>

namespace wind::client {
namespace {

// 테마에 적힌 글꼴 하나를 불러와요. 실패하면 기록만 하고 넘어가요.
void load_font(engine::TextRenderer& text, engine::FontStyle style, const engine::UiFont& font,
               const std::filesystem::path& content_root) {
    if (font.file.empty()) {
        return;
    }
    const auto file = content::resolve_content_path(content_root, font.file);
    std::string error;
    if (!file || !text.load_font(style, *file, font.pixel_size, error)) {
        log::error("Cannot load font '{}': {}", font.file, file ? error : "invalid content path");
        return;
    }
    log::info("Font '{}' loaded at {} px", font.file, font.pixel_size);
}

}  // namespace

UiResources load_ui_resources(engine::ResourceManager& resources, engine::TextRenderer& text,
                              std::string_view theme_path) {
    UiResources ui;
    auto theme = engine::load_ui_theme_file(resources.content_root(), theme_path);
    if (theme.ok()) {
        ui.theme = std::move(*theme.value);
    } else {
        content::log_content_errors("UI theme", theme.errors);
        log::warn("Using the built-in UI look (no frames, no fonts)");
        return ui;
    }
    load_font(text, engine::FontStyle::Regular, ui.theme.regular, resources.content_root());
    load_font(text, engine::FontStyle::Bold, ui.theme.bold, resources.content_root());
    if (!ui.theme.image.empty()) {
        ui.frames = resources.texture(ui.theme.image);
    }
    return ui;
}

}  // namespace wind::client
