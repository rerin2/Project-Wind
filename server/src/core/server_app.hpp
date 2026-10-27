#pragma once

// 서버의 메인 루프를 소유하는 클래스.

#include "core/server_options.hpp"

#include <atomic>
#include <cstdint>

namespace wind::server {

// 단일 논리 월드 루프(명세 3.4)를 실행한다. 월드 상태 변경은 모두 이 루프 안에서 일어난다.
// 단계 0에서는 빈 틱만 고정 주기로 돌고, 이후 단계에서 tick()에 네트워크·월드 갱신이 추가된다.
class ServerApp {
public:
    explicit ServerApp(ServerOptions options);

    // stop_requested가 true가 되거나 max_ticks에 도달할 때까지 실행한다.
    // 반환값은 프로세스 종료 코드다 (정상 종료 0, 초기화 실패 1).
    int run(const std::atomic<bool>& stop_requested);

    // 실행된 틱 수와, 틱 하나가 예산 시간을 넘긴 횟수 (테스트·계측용)
    [[nodiscard]] std::uint64_t ticks_executed() const { return ticks_executed_; }
    [[nodiscard]] std::uint64_t tick_overruns() const { return tick_overruns_; }

private:
    // 틱 한 번 동안 할 일. 현재는 비어 있다.
    void tick();

    ServerOptions options_;
    std::uint64_t ticks_executed_ = 0;
    std::uint64_t tick_overruns_ = 0;
};

}  // namespace wind::server
