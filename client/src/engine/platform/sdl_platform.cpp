// 이 파일은 무엇을 하나요?
// 통역사(SdlPlatform)가 SDL로 창을 만들고, 키보드 소식을 받아 우리 말로 옮기는 곳이에요.

#include "engine/platform/sdl_platform.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

namespace wind::engine {
namespace {

// SDL의 "스캔코드"를 우리 엔진의 Key로 바꿔요.
// 스캔코드 = 키보드 위 "자리" 번호예요. 글자가 아니라 위치라서,
// 한글/영어 자판을 바꿔도 W·A·S·D 자리는 그대로예요. 그래서 이동 키로 쓰기 좋아요.
Key translate(SDL_Scancode code) {
    switch (code) {
        case SDL_SCANCODE_UP: return Key::Up;
        case SDL_SCANCODE_DOWN: return Key::Down;
        case SDL_SCANCODE_LEFT: return Key::Left;
        case SDL_SCANCODE_RIGHT: return Key::Right;
        case SDL_SCANCODE_W: return Key::W;
        case SDL_SCANCODE_A: return Key::A;
        case SDL_SCANCODE_S: return Key::S;
        case SDL_SCANCODE_D: return Key::D;
        case SDL_SCANCODE_ESCAPE: return Key::Escape;
        case SDL_SCANCODE_RETURN: return Key::Enter;
        case SDL_SCANCODE_SPACE: return Key::Space;
        case SDL_SCANCODE_K: return Key::K;
        case SDL_SCANCODE_X: return Key::X;
        case SDL_SCANCODE_F3: return Key::F3;
        case SDL_SCANCODE_F4: return Key::F4;
        default: return Key::Unknown;  // 우리 게임이 안 쓰는 키
    }
}

}  // namespace

std::unique_ptr<SdlPlatform> SdlPlatform::create(const WindowConfig& config) {
    // SDL 시작하기 (영상 기능만). SDL3 함수는 성공하면 true를 돌려줘요.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log::critical("SDL_Init failed: {}", SDL_GetError());
        return nullptr;
    }

    // 여기부터는 중간에 실패해도, 이 상자가 사라질 때 이미 만든 것들을 알아서 정리해요.
    std::unique_ptr<SdlPlatform> platform(new SdlPlatform());

    // 창 만들기. RESIZABLE = 사용자가 창 크기를 바꿀 수 있어요.
    platform->window_ =
        SDL_CreateWindow(config.title.c_str(), static_cast<int>(config.width),
                         static_cast<int>(config.height), SDL_WINDOW_RESIZABLE);
    if (platform->window_ == nullptr) {
        log::critical("SDL_CreateWindow failed: {}", SDL_GetError());
        return nullptr;
    }

    // 붓 만들기. nullptr = "알아서 제일 좋은 걸로 골라 줘" (Windows에서는 보통 Direct3D 11)
    platform->sdl_renderer_ = SDL_CreateRenderer(platform->window_, nullptr);
    if (platform->sdl_renderer_ == nullptr) {
        log::critical("SDL_CreateRenderer failed: {}", SDL_GetError());
        return nullptr;
    }

    // VSync(수직 동기화): 모니터가 화면을 바꾸는 박자에 맞춰 그려요. 화면이 찢어져 보이지 않아요.
    // 안 돼도 게임은 할 수 있어서 경고만 남겨요.
    if (!SDL_SetRenderVSync(platform->sdl_renderer_, 1)) {
        log::warn("VSync unavailable: {}", SDL_GetError());
    }

    // 우리 엔진의 화가를 만들고, 게임 화면 크기(640x360 등)를 정해요.
    platform->renderer_ = std::make_unique<Renderer>(platform->sdl_renderer_);
    if (!platform->renderer_->set_logical_size(config.logical_width, config.logical_height)) {
        return nullptr;
    }

    // 준비 완료! 어떤 SDL 버전과 붓을 쓰는지 기록해요.
    const int version = SDL_GetVersion();
    log::info("SDL {}.{}.{} initialized, renderer '{}', logical size {}x{}",
              SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version),
              SDL_VERSIONNUM_MICRO(version), SDL_GetRendererName(platform->sdl_renderer_),
              config.logical_width, config.logical_height);
    return platform;
}

SdlPlatform::~SdlPlatform() {
    // 엔진 화가가 가진 스티커 종이(텍스처)는 SDL 붓으로 만든 거예요.
    // 붓을 먼저 버리면 스티커 종이를 치울 방법이 없어지니까, 화가(와 스티커)를 먼저 정리해요.
    renderer_.reset();
    if (sdl_renderer_ != nullptr) {
        SDL_DestroyRenderer(sdl_renderer_);
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();  // 마지막으로 SDL 끝내기
}

bool SdlPlatform::pump_events(InputState& input) {
    // 쌓인 소식(이벤트)을 매번 전부 꺼내 봐야 해요.
    // 안 그러면 Windows가 "응답 없음"이라고 생각해요.
    SDL_Event event;
    bool keep_running = true;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                keep_running = false;  // 창 닫기 버튼을 눌렀어요
                break;
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                // 키를 꾹 누르고 있으면 컴퓨터가 "또 눌림"을 계속 보내요(자동 반복).
                // 우리는 "누르고 있음" 상태로 충분하니까 그 반복 소식은 무시해요.
                if (!event.key.repeat) {
                    input.set_key(translate(event.key.scancode), event.type == SDL_EVENT_KEY_DOWN);
                }
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                // 다른 창으로 옮겨 가면 키를 뗀 소식을 못 받을 수 있어서, 모든 키를 뗀 것으로 해요.
                input.release_all();
                break;
            default:
                break;  // 나머지 소식은 아직 쓰지 않아요
        }
    }
    return keep_running;
}

}  // namespace wind::engine
