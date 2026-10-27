#include "core/server_app.hpp"

#include <gtest/gtest.h>

#include <atomic>

namespace {

using wind::server::ServerApp;
using wind::server::ServerOptions;

TEST(ServerApp, StopsAfterMaxTicks) {
    ServerOptions options;
    options.tick_rate_hz = 100;
    options.max_ticks = 3;

    ServerApp app(options);
    const std::atomic<bool> stop{false};
    EXPECT_EQ(app.run(stop), 0);
    EXPECT_EQ(app.ticks_executed(), 3u);
}

TEST(ServerApp, StopsImmediatelyWhenStopAlreadyRequested) {
    ServerOptions options;
    options.tick_rate_hz = 100;

    ServerApp app(options);
    const std::atomic<bool> stop{true};
    EXPECT_EQ(app.run(stop), 0);
    EXPECT_EQ(app.ticks_executed(), 0u);
}

}  // namespace
