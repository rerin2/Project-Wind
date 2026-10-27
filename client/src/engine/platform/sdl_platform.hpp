#pragma once

// SDL 전역 상태, OS 창, SDL 렌더러의 수명을 RAII로 관리하는 플랫폼 계층.
// SDL 헤더는 엔진 코드에서만 포함한다. 게임 코드는 엔진 인터페이스만 사용한다.

#include <cstdint>
#include <memory>
#include <string>

// SDL 헤더를 노출하지 않도록 전방 선언만 한다.
struct SDL_Window;
struct SDL_Renderer;

namespace wind::engine {

// 창 생성 설정
struct WindowConfig {
    std::string title;
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

class SdlPlatform {
public:
    // SDL 초기화 -> 창 생성 -> 렌더러 생성을 차례로 수행한다.
    // 어느 단계든 실패하면 SDL 오류를 로그로 남기고 nullptr를 돌려준다.
    [[nodiscard]] static std::unique_ptr<SdlPlatform> create(const WindowConfig& config);

    // 렌더러 -> 창 -> SDL 순서(생성의 역순)로 해제한다.
    ~SdlPlatform();
    SdlPlatform(const SdlPlatform&) = delete;
    SdlPlatform& operator=(const SdlPlatform&) = delete;

    // 쌓인 OS 이벤트를 모두 처리한다. 사용자가 창을 닫으면 false를 돌려준다.
    [[nodiscard]] bool pump_events();

    // 프레임 시작 시 화면을 지우고, 끝에 그린 결과를 화면에 표시한다.
    void begin_frame();
    void end_frame();

private:
    SdlPlatform() = default;  // create()로만 생성한다

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
};

}  // namespace wind::engine
