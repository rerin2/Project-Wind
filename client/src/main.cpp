// 클라이언트 진입점: 인자 파싱 -> 로그 초기화 -> ClientApp 실행.
#include "app/client_app.hpp"
#include "app/client_options.hpp"

#include "wind/cli.hpp"
#include "wind/log.hpp"

// 플랫폼별 진입점(Windows의 WinMain 등)을 SDL이 표준 main()으로 연결해 준다.
#include <SDL3/SDL_main.h>

#include <iostream>
#include <span>

int main(int argc, char** argv) {
    // argv[0](프로그램 이름)을 제외하고 파싱한다.
    const auto args = wind::cli::to_args(argc, argv);
    const auto parsed =
        wind::client::parse_client_options(std::span(args).subspan(args.empty() ? 0 : 1));

    if (parsed.show_help) {
        std::cout << wind::client::client_usage();
        return 0;
    }
    if (!parsed.options) {
        // 잘못된 인자: 오류와 사용법을 출력하고 종료 코드 2로 끝낸다.
        std::cerr << "error: " << parsed.error << "\n\n" << wind::client::client_usage();
        return 2;
    }

    wind::log::init({.component = "client", .level = parsed.options->log_level});

    wind::client::ClientApp app(*parsed.options);
    const int exit_code = app.run();

    wind::log::shutdown();  // 남은 로그를 모두 출력한 뒤 종료
    return exit_code;
}
