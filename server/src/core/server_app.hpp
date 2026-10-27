#pragma once

#include "core/server_options.hpp"

#include <atomic>
#include <cstdint>

namespace wind::server {

// Owns the single logical world loop (master spec 3.4). Phase 0 runs an empty
// fixed-rate tick; later phases add networking and world updates to tick().
class ServerApp {
public:
    explicit ServerApp(ServerOptions options);

    // Runs until stop_requested becomes true or max_ticks is reached.
    // Returns a process exit code: 0 on clean shutdown.
    int run(const std::atomic<bool>& stop_requested);

    [[nodiscard]] std::uint64_t ticks_executed() const { return ticks_executed_; }
    [[nodiscard]] std::uint64_t tick_overruns() const { return tick_overruns_; }

private:
    void tick();

    ServerOptions options_;
    std::uint64_t ticks_executed_ = 0;
    std::uint64_t tick_overruns_ = 0;
};

}  // namespace wind::server
