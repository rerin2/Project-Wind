#include "app/client_app.hpp"

#include "engine/platform/sdl_platform.hpp"

#include "wind/log.hpp"
#include "wind/version.hpp"

#include <string>

namespace wind::client {

ClientApp::ClientApp(ClientOptions options) : options_(options) {}

int ClientApp::run() {
    log::info("{} client {} starting", kGameName, kVersion);

    // 창과 렌더러 준비. 실패 원인은 SdlPlatform::create가 이미 로그로 남겼다.
    auto platform = engine::SdlPlatform::create({
        .title = std::string(kGameName),
        .width = options_.window_width,
        .height = options_.window_height,
    });
    if (!platform) {
        log::critical("Client failed to initialize the platform layer");
        return 1;
    }

    // 프레임 루프: 이벤트 처리 -> 화면 지우기 -> 표시. 창을 닫으면 끝난다.
    while (platform->pump_events()) {
        if (options_.max_frames && frames_rendered_ >= *options_.max_frames) {
            log::info("Reached --max-frames {}", *options_.max_frames);
            break;
        }
        platform->begin_frame();
        platform->end_frame();
        ++frames_rendered_;
    }

    log::info("Client stopping after {} frames", frames_rendered_);
    platform.reset();  // SDL 자원을 여기서 명시적으로 해제한다
    log::info("Client stopped cleanly");
    return 0;
}

}  // namespace wind::client
