// 이 파일은 무엇을 하나요?
// SDL_ttf로 글꼴을 읽고, 글 폭을 재고, 글을 그리는 곳이에요.

#include "engine/render/text_renderer.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cmath>

namespace wind::engine {
namespace {

// 보관함에 이보다 많은 글이 쌓이면 한 번 비워요. (글이 계속 바뀌는 숫자 표시 등으로 끝없이 늘지 않게)
constexpr std::size_t kMaxCachedTexts = 512;
// 한 번에 그릴 수 있는 글 길이 (바이트). 이상하게 긴 글이 들어와도 멈추지 않게 잘라요.
constexpr std::size_t kMaxTextBytes = 4096;

std::size_t slot_index(FontStyle style) {
    return static_cast<std::size_t>(style);
}

std::string_view clamp_text(std::string_view text) {
    return text.size() > kMaxTextBytes ? text.substr(0, kMaxTextBytes) : text;
}

}  // namespace

TextRenderer::TextRenderer(SDL_Renderer* renderer)
    : engine_(TTF_CreateRendererTextEngine(renderer)) {
    if (engine_ == nullptr) {
        log::error("TTF_CreateRendererTextEngine failed: {}", SDL_GetError());
    }
}

TextRenderer::~TextRenderer() {
    // 글 조각 -> 글꼴 -> 글 엔진 순서로 정리해요 (만든 순서의 반대).
    for (auto& slot : fonts_) {
        clear_cache(slot);
        if (slot.font != nullptr) {
            TTF_CloseFont(slot.font);
        }
    }
    if (engine_ != nullptr) {
        TTF_DestroyRendererTextEngine(engine_);
    }
}

bool TextRenderer::load_font(FontStyle style, const std::filesystem::path& file, float pixel_size,
                             std::string& error) {
    if (style == FontStyle::Count) {
        error = "invalid font style";
        return false;
    }
    auto& slot = fonts_[slot_index(style)];
    TTF_Font* font = TTF_OpenFont(file.string().c_str(), pixel_size);
    if (font == nullptr) {
        error = SDL_GetError();
        return false;
    }
    // 흑백 힌팅: 반투명한 가장자리 없이 또렷한 도트 글자
    TTF_SetFontHinting(font, TTF_HINTING_MONO);
    clear_cache(slot);
    if (slot.font != nullptr) {
        TTF_CloseFont(slot.font);
    }
    slot.font = font;
    return true;
}

bool TextRenderer::ready(FontStyle style) const {
    return style != FontStyle::Count && engine_ != nullptr && fonts_[slot_index(style)].font != nullptr;
}

float TextRenderer::measure(FontStyle style, std::string_view text) const {
    if (!ready(style) || text.empty()) {
        return 0.0f;
    }
    text = clamp_text(text);
    int width = 0;
    int height = 0;
    if (!TTF_GetStringSize(fonts_[slot_index(style)].font, text.data(), text.size(), &width, &height)) {
        return 0.0f;
    }
    return static_cast<float>(width);
}

float TextRenderer::line_height(FontStyle style) const {
    if (!ready(style)) {
        return 0.0f;
    }
    return static_cast<float>(TTF_GetFontHeight(fonts_[slot_index(style)].font));
}

MeasureText TextRenderer::measurer(FontStyle style) const {
    return [this, style](std::string_view text) { return measure(style, text); };
}

void TextRenderer::draw(FontStyle style, Vec2 position, std::string_view text, Color color) {
    if (!ready(style) || text.empty()) {
        return;
    }
    TTF_Text* drawn = cached_text(fonts_[slot_index(style)], clamp_text(text));
    if (drawn == nullptr) {
        return;
    }
    TTF_SetTextColor(drawn, color.r, color.g, color.b, color.a);
    // 정수 픽셀에 맞춰야 도트 글자가 번지지 않아요.
    if (!TTF_DrawRendererText(drawn, std::round(position.x), std::round(position.y))) {
        log::debug("TTF_DrawRendererText failed: {}", SDL_GetError());
    }
    ++draws_;
}

std::size_t TextRenderer::cached() const {
    std::size_t total = 0;
    for (const auto& slot : fonts_) {
        total += slot.cache.size();
    }
    return total;
}

TTF_Text* TextRenderer::cached_text(FontSlot& slot, std::string_view text) {
    const std::string key(text);
    if (const auto it = slot.cache.find(key); it != slot.cache.end()) {
        return it->second;
    }
    if (slot.cache.size() >= kMaxCachedTexts) {
        clear_cache(slot);
    }
    TTF_Text* created = TTF_CreateText(engine_, slot.font, key.data(), key.size());
    if (created == nullptr) {
        log::debug("TTF_CreateText failed: {}", SDL_GetError());
        return nullptr;
    }
    slot.cache.emplace(key, created);
    return created;
}

void TextRenderer::clear_cache(FontSlot& slot) {
    for (auto& [text, created] : slot.cache) {
        TTF_DestroyText(created);
    }
    slot.cache.clear();
}

}  // namespace wind::engine
