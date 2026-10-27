#include "core/server_app.hpp"

#include "wind/log.hpp"
#include "wind/version.hpp"

#include <enet/enet.h>

#include <chrono>
#include <thread>

namespace wind::server {
namespace {

using Clock = std::chrono::steady_clock;

// RAII guard for ENet's global state.
class EnetRuntime {
public:
    EnetRuntime() : ok_(enet_initialize() == 0) {}
    ~EnetRuntime() {
        if (ok_) {
            enet_deinitialize();
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

    EnetRuntime enet;
    if (!enet.ok()) {
        log::critical("ENet initialization failed; server cannot start");
        return 1;
    }
    log::info("ENet {}.{}.{} initialized", ENET_VERSION_MAJOR, ENET_VERSION_MINOR,
              ENET_VERSION_PATCH);

    const auto tick_interval = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(1.0 / options_.tick_rate_hz));
    auto next_tick = Clock::now();

    log::info("Server running; press Ctrl+C to stop");
    while (!stop_requested.load()) {
        if (options_.max_ticks && ticks_executed_ >= *options_.max_ticks) {
            log::info("Reached --max-ticks {}", *options_.max_ticks);
            break;
        }

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

        next_tick += tick_interval;
        const auto now = Clock::now();
        if (next_tick < now) {
            // Fell behind: resume from now rather than running catch-up ticks.
            next_tick = now;
        }
        std::this_thread::sleep_until(next_tick);
    }

    log::info("Server stopping after {} ticks ({} overruns)", ticks_executed_, tick_overruns_);
    log::info("Server stopped cleanly");
    return 0;
}

void ServerApp::tick() {
    // World update, network pump, and persistence hooks are added in later phases.
}

}  // namespace wind::server
