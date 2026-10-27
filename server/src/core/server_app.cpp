#include "core/server_app.hpp"

#include "wind/log.hpp"
#include "wind/version.hpp"

#include <enet/enet.h>

#include <chrono>
#include <thread>

namespace wind::server {
namespace {

// 시스템 시계 변경에 영향받지 않는 단조 증가 시계로 틱 간격을 잰다.
using Clock = std::chrono::steady_clock;

// ENet 전역 상태의 초기화·해제를 묶은 RAII 가드.
// 생성 시 enet_initialize(), 소멸 시 enet_deinitialize()를 호출한다.
class EnetRuntime {
public:
    EnetRuntime() : ok_(enet_initialize() == 0) {}
    ~EnetRuntime() {
        if (ok_) {
            enet_deinitialize();  // 초기화에 성공했을 때만 해제한다
        }
    }
    EnetRuntime(const EnetRuntime&) = delete;
    EnetRuntime& operator=(const EnetRuntime&) = delete;

    [[nodiscard]] bool ok() const { return ok_; }

private:
    bool ok_;
};

}  // namespace

ServerApp::ServerApp(ServerOptions options) : options_(options) {}

int ServerApp::run(const std::atomic<bool>& stop_requested) {
    log::info("{} server {} starting (tick rate {} Hz)", kGameName, kVersion,
              options_.tick_rate_hz);

    // ENet이 없으면 서버가 동작할 수 없으므로 바로 실패로 끝낸다.
    EnetRuntime enet;
    if (!enet.ok()) {
        log::critical("ENet initialization failed; server cannot start");
        return 1;
    }
    log::info("ENet {}.{}.{} initialized", ENET_VERSION_MAJOR, ENET_VERSION_MINOR,
              ENET_VERSION_PATCH);

    // 틱 간격 = 1초 / 틱 수 (20 Hz면 50 ms)
    const auto tick_interval = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(1.0 / options_.tick_rate_hz));
    auto next_tick = Clock::now();  // 다음 틱을 시작할 예정 시각

    log::info("Server running; press Ctrl+C to stop");
    while (!stop_requested.load()) {
        if (options_.max_ticks && ticks_executed_ >= *options_.max_ticks) {
            log::info("Reached --max-ticks {}", *options_.max_ticks);
            break;
        }

        // 틱 실행 시간을 재서 예산(틱 간격)을 넘기면 경고로 남긴다.
        const auto tick_start = Clock::now();
        tick();
        ++ticks_executed_;
        const auto tick_duration = Clock::now() - tick_start;

        if (tick_duration > tick_interval) {
            ++tick_overruns_;
            log::warn("Tick {} took {} us, budget {} us", ticks_executed_,
                      std::chrono::duration_cast<std::chrono::microseconds>(tick_duration).count(),
                      std::chrono::duration_cast<std::chrono::microseconds>(tick_interval).count());
        }

        // 고정 간격 스케줄: 예정 시각을 한 칸 앞으로 옮긴다 (드리프트 누적 방지).
        next_tick += tick_interval;
        const auto now = Clock::now();
        if (next_tick < now) {
            // 늦어졌으면 밀린 틱을 몰아서 실행하지 않고 지금부터 다시 시작한다.
            next_tick = now;
        }
        std::this_thread::sleep_until(next_tick);
    }

    log::info("Server stopping after {} ticks ({} overruns)", ticks_executed_, tick_overruns_);
    log::info("Server stopped cleanly");
    return 0;  // 여기서 EnetRuntime 소멸자가 ENet을 해제한다
}

void ServerApp::tick() {
    // 월드 갱신, 네트워크 처리, 저장 작업은 이후 단계에서 이곳에 추가한다.
}

}  // namespace wind::server
