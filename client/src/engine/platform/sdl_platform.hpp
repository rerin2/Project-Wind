#pragma once

// RAII ownership of SDL's global state, the OS window, and the SDL renderer.
// Only engine code may include SDL headers; game code talks to engine interfaces.

#include <cstdint>
#include <memory>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

namespace wind::engine {

struct WindowConfig {
    std::string title;
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

class SdlPlatform {
public:
    // Returns nullptr and logs the SDL error if any step fails.
    [[nodiscard]] static std::unique_ptr<SdlPlatform> create(const WindowConfig& config);

    ~SdlPlatform();
    SdlPlatform(const SdlPlatform&) = delete;
    SdlPlatform& operator=(const SdlPlatform&) = delete;

    // Drains OS events. Returns false once the user asked to quit.
    [[nodiscard]] bool pump_events();

    void begin_frame();
    void end_frame();

private:
    SdlPlatform() = default;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
};

}  // namespace wind::engine
