#pragma once

// 이 파일은 무엇을 하나요?
// 플랫폼 계층 = 운영체제(Windows)와 이야기하는 "통역사"예요.
// SDL(창·키보드·그래픽을 다루는 도구 상자)을 시작하고, 게임 창과 그림 붓(렌더러)을 만들어요.
// RAII(쓴 물건은 자동으로 제자리에) 방식이라, 이 물건이 사라질 때 창과 붓도 알아서 정리돼요.
// SDL 헤더는 엔진 코드 안에서만 써요. 게임 코드는 엔진이 주는 도구만 써요.

#include "engine/input/input_state.hpp"
#include "engine/render/renderer.hpp"

#include <cstdint>
#include <memory>
#include <string>

// SDL의 속 모습은 숨기고 이름만 알려 줘요.
struct SDL_Window;
struct SDL_Renderer;

namespace wind::engine {

// 창을 만들 때 쓰는 설정
struct WindowConfig {
    std::string title;           // 창 제목
    std::uint32_t width = 1280;  // 창의 실제 크기
    std::uint32_t height = 720;
    // 게임 화면의 기본 크기(논리 해상도). 창 크기에 맞춰 2배, 3배처럼 정수 배로 커져요.
    std::int32_t logical_width = 640;
    std::int32_t logical_height = 360;
};

class SdlPlatform {
public:
    // SDL 시작 -> 창 만들기 -> 붓 만들기를 차례로 해요.
    // 중간에 하나라도 실패하면 이유를 기록하고 nullptr(빈손)를 돌려줘요.
    [[nodiscard]] static std::unique_ptr<SdlPlatform> create(const WindowConfig& config);

    // 만든 순서의 반대로 정리해요: 엔진 화가 -> SDL 붓 -> 창 -> SDL 끝내기
    // (양말을 신고 신발을 신었으면, 벗을 때는 신발부터 벗는 것과 같아요)
    ~SdlPlatform();
    SdlPlatform(const SdlPlatform&) = delete;
    SdlPlatform& operator=(const SdlPlatform&) = delete;

    // 창에서 일어난 일(키 누름, 창 닫기 등)을 모두 확인하고 키보드 출석부(input)에 적어요.
    // 사용자가 창을 닫으면 false를 돌려줘요.
    [[nodiscard]] bool pump_events(InputState& input);

    // 그림 그리는 화가를 빌려줘요.
    [[nodiscard]] Renderer& renderer() { return *renderer_; }

private:
    SdlPlatform() = default;  // create()로만 만들 수 있어요 (실패 확인을 빼먹지 않도록)

    SDL_Window* window_ = nullptr;          // 게임 창
    SDL_Renderer* sdl_renderer_ = nullptr;  // SDL 붓
    std::unique_ptr<Renderer> renderer_;    // 우리 엔진의 화가 (SDL 붓을 빌려 씀)
};

}  // namespace wind::engine
