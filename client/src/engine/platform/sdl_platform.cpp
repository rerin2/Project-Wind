#include "engine/platform/sdl_platform.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

namespace wind::engine {

std::unique_ptr<SdlPlatform> SdlPlatform::create(const WindowConfig& config) {
    // SDL3 함수는 성공 시 true를 돌려준다.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log::critical("SDL_Init failed: {}", SDL_GetError());
        return nullptr;
    }

    // 여기서부터는 중간에 실패해도 소멸자가 이미 만든 자원을 정리한다.
    std::unique_ptr<SdlPlatform> platform(new SdlPlatform());

    platform->window_ =
        SDL_CreateWindow(config.title.c_str(), static_cast<int>(config.width),
                         static_cast<int>(config.height), SDL_WINDOW_RESIZABLE);
    if (platform->window_ == nullptr) {
        log::critical("SDL_CreateWindow failed: {}", SDL_GetError());
        return nullptr;
    }

    // nullptr: SDL이 가장 적합한 렌더러를 고른다 (Windows에서는 보통 Direct3D 11).
    platform->renderer_ = SDL_CreateRenderer(platform->window_, nullptr);
    if (platform->renderer_ == nullptr) {
        log::critical("SDL_CreateRenderer failed: {}", SDL_GetError());
        return nullptr;
    }

    // VSync로 모니터 주사율에 맞춰 그린다. 실패해도 실행에는 지장이 없어 경고만 남긴다.
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
    // 생성의 역순으로 해제한다. 만들어지지 않은 자원은 건너뛴다.
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool SdlPlatform::pump_events() {
    // 이벤트 큐를 매 프레임 모두 비워야 창이 "응답 없음" 상태가 되지 않는다.
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
    // 어두운 남색 배경으로 화면을 지운다.
    SDL_SetRenderDrawColor(renderer_, 24, 32, 40, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer_);
}

void SdlPlatform::end_frame() {
    SDL_RenderPresent(renderer_);  // 백버퍼를 화면에 표시
}

}  // namespace wind::engine
