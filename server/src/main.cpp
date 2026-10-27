// 서버 진입점: 인자 파싱 -> 로그 초기화 -> 종료 신호 등록 -> ServerApp 실행.
#include "core/server_app.hpp"
#include "core/server_options.hpp"

#include "wind/cli.hpp"
#include "wind/log.hpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <span>

namespace {

// 신호 처리기가 설정하고 메인 루프가 읽는 종료 요청 플래그.
// lock-free atomic은 신호 처리기 안에서 안전하게 쓸 수 있다.
std::atomic<bool> g_stop_requested{false};
static_assert(std::atomic<bool>::is_always_lock_free);

// Ctrl+C 등이 오면 플래그만 세운다. 실제 정리는 메인 루프가 한다.
extern "C" void handle_stop_signal(int /*signal*/) {
    g_stop_requested.store(true);
}

// Ctrl+C(SIGINT), 종료 요청(SIGTERM), Windows의 Ctrl+Break(SIGBREAK)를 처리한다.
void install_signal_handlers() {
    std::signal(SIGINT, handle_stop_signal);
    std::signal(SIGTERM, handle_stop_signal);
#ifdef SIGBREAK
    std::signal(SIGBREAK, handle_stop_signal);
#endif
}

}  // namespace

int main(int argc, char** argv) {
    // argv[0](프로그램 이름)을 제외하고 파싱한다.
    const auto args = wind::cli::to_args(argc, argv);
    const auto parsed =
        wind::server::parse_server_options(std::span(args).subspan(args.empty() ? 0 : 1));

    if (parsed.show_help) {
        std::cout << wind::server::server_usage();
        return 0;
    }
    if (!parsed.options) {
        // 잘못된 인자: 오류와 사용법을 출력하고 종료 코드 2로 끝낸다.
        std::cerr << "error: " << parsed.error << "\n\n" << wind::server::server_usage();
        return 2;
    }

    wind::log::init({.component = "server", .level = parsed.options->log_level});
    install_signal_handlers();

    wind::server::ServerApp app(*parsed.options);
    const int exit_code = app.run(g_stop_requested);

    wind::log::shutdown();  // 남은 로그를 모두 출력한 뒤 종료
    return exit_code;
}
