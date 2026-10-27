#include "engine/platform/sdl_platform.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

namespace wind::engine {

std::unique_ptr<SdlPlatform> SdlPlatform::create(const WindowConfig& config) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log::critical("SDL_Init failed: {}", SDL_GetError());
        return nullptr;
    }

    // From here on the destructor cleans up whatever was created.
    std::unique_ptr<SdlPlatform> platform(new SdlPlatform());

    platform->window_ =
        SDL_CreateWindow(config.title.c_str(), static_cast<int>(config.width),
                         static_cast<int>(config.height), SDL_WINDOW_RESIZABLE);
    if (platform->window_ == nullptr) {
        log::critical("SDL_CreateWindow failed: {}", SDL_GetError());
        return nullptr;
    }

    platform->renderer_ = SDL_CreateRenderer(platform->window_, nullptr);
    if (platform->renderer_ == nullptr) {
        log::critical("SDL_CreateRenderer failed: {}", SDL_GetError());
        return nullptr;
    }

    if (!SDL_SetRenderVSync(platform->renderer_, 1)) {
        log::warn("VSync unavailable: {}", SDL_GetError());
    }

    const int version = SDL_GetVersion();
    log::info("SDL {}.{}.{} initialized, renderer '{}'", SDL_VERSIONNUM_MAJOR(version),
              SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version),
              SDL_GetRendererName(platform->renderer_));
    return platform;
}

SdlPlatform::~SdlPlatform() {
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool SdlPlatform::pump_events() {
    SDL_Event event;
    bool keep_running = true;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            keep_running = false;
        }
    }
    return keep_running;
}

void SdlPlatform::begin_frame() {
    SDL_SetRenderDrawColor(renderer_, 24, 32, 40, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer_);
}

void SdlPlatform::end_frame() {
    SDL_RenderPresent(renderer_);
}

}  // namespace wind::engine
