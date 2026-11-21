// 이 파일은 무엇을 하나요?
// 도화지 픽셀을 CPU로 읽어 와서 BMP로 저장하거나(스크린샷), 흑백으로 바꿔 다시 그리는(12b) 곳이에요.

#include "engine/render/frame_readback.hpp"

#include "engine/render/desaturate.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <system_error>

namespace wind::engine {
namespace {

// 서피스(CPU 그림)를 다 쓰면 저절로 버려 주는 상자 (RAII)
struct SurfaceDeleter {
    void operator()(SDL_Surface* surface) const { SDL_DestroySurface(surface); }
};
using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

}  // namespace

void save_frame_bmp(SDL_Renderer* renderer, const std::filesystem::path& path) {
    // 저장할 폴더가 없으면 만들어요.
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        log::error("Cannot create screenshot folder {}: {}", path.parent_path().string(), ec.message());
        return;
    }
    // 지금 도화지의 픽셀을 그래픽 카드에서 읽어 와요.
    const SurfacePtr surface(SDL_RenderReadPixels(renderer, nullptr));
    if (!surface) {
        log::error("Cannot read pixels for screenshot: {}", SDL_GetError());
        return;
    }
    const auto utf8 = path.u8string();  // SDL은 UTF-8 경로를 받아요 (한국어 폴더도 괜찮게)
    if (SDL_SaveBMP(surface.get(), reinterpret_cast<const char*>(utf8.c_str()))) {
        log::info("Saved screenshot {} ({}x{})", path.string(), surface->w, surface->h);
    } else {
        log::error("Cannot save screenshot {}: {}", path.string(), SDL_GetError());
    }
}

bool desaturate_frame(SDL_Renderer* renderer, SDL_Texture* scratch, float amount) {
    if (scratch == nullptr) {
        return false;  // 흑백용 텍스처를 못 만들었어요 (만들 때 이미 기록했어요)
    }
    // 1) 지금 도화지를 읽어 와요 (그래픽 카드 -> CPU)
    const SurfacePtr raw(SDL_RenderReadPixels(renderer, nullptr));
    if (!raw) {
        log::error("Cannot read pixels for grayscale: {}", SDL_GetError());
        return false;
    }
    // 2) 바이트 순서를 R, G, B, A로 맞춰요 (그래픽 카드마다 순서가 달라서)
    const SurfacePtr rgba(SDL_ConvertSurface(raw.get(), SDL_PIXELFORMAT_RGBA32));
    if (!rgba) {
        log::error("Cannot convert pixels for grayscale: {}", SDL_GetError());
        return false;
    }
    // 3) 줄마다 회색 쪽으로 바꿔요. 한 줄의 바이트 수(pitch)는 가로 × 4보다 클 수 있어서 줄씩 해요.
    auto* bytes = static_cast<std::uint8_t*>(rgba->pixels);
    const auto row_bytes = static_cast<std::size_t>(rgba->w) * 4;
    for (int y = 0; y < rgba->h; ++y) {
        desaturate_rgba(std::span<std::uint8_t>(bytes + static_cast<std::size_t>(y) * rgba->pitch, row_bytes),
                        amount);
    }
    // 4) 바꾼 그림을 그래픽 카드에 다시 올리고, 도화지 전체에 그대로 덮어 그려요
    if (!SDL_UpdateTexture(scratch, nullptr, rgba->pixels, rgba->pitch)) {
        log::error("Cannot upload grayscale pixels: {}", SDL_GetError());
        return false;
    }
    return SDL_RenderTexture(renderer, scratch, nullptr, nullptr);
}

}  // namespace wind::engine
