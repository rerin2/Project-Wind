// ServerApp 루프의 종료 조건을 검사한다. 테스트가 빨리 끝나도록 100 Hz로 돌린다.
#include "core/server_app.hpp"

#include <gtest/gtest.h>

#include <atomic>

namespace {

using wind::server::ServerApp;
using wind::server::ServerOptions;

// max_ticks에 도달하면 정확히 그 틱 수만큼 돌고 정상 종료(0)한다.
TEST(ServerApp, StopsAfterMaxTicks) {
    ServerOptions options;
    options.tick_rate_hz = 100;
    options.max_ticks = 3;

    ServerApp app(options);
    const std::atomic<bool> stop{false};
    EXPECT_EQ(app.run(stop), 0);
    EXPECT_EQ(app.ticks_executed(), 3u);
}

// 시작 전에 이미 종료가 요청됐다면 틱을 하나도 돌지 않고 끝난다.
TEST(ServerApp, StopsImmediatelyWhenStopAlreadyRequested) {
    ServerOptions options;
    options.tick_rate_hz = 100;

    ServerApp app(options);
    const std::atomic<bool> stop{true};
    EXPECT_EQ(app.run(stop), 0);
    EXPECT_EQ(app.ticks_executed(), 0u);
}

}  // namespace
