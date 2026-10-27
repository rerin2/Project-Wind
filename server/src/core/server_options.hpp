#pragma once

// 서버 실행 옵션과 명령줄 파싱.

#include "wind/log.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace wind::server {

// 서버 실행 설정. 기본값만으로도 정상 실행된다.
struct ServerOptions {
    // 월드 갱신 주기(초당 틱 수). 근거: docs/DECISIONS/0004-server-tick-rate.md
    std::uint32_t tick_rate_hz = 20;
    // 이 틱 수만큼 돈 뒤 종료한다. 비어 있으면 종료 요청이 올 때까지 계속 실행한다.
    std::optional<std::uint64_t> max_ticks;
    log::Level log_level = log::Level::Info;
};

// --tick-rate로 허용하는 범위
inline constexpr std::uint32_t kMinTickRateHz = 1;
inline constexpr std::uint32_t kMaxTickRateHz = 120;

// 파싱 결과. 성공·실패·도움말 요청 중 하나다.
struct ParseResult {
    std::optional<ServerOptions> options;  // 성공 시 설정됨
    std::string error;                     // 실패 시 사람이 읽을 오류 메시지
    bool show_help = false;                // --help 요청 시 true
};

// 명령줄 인자를 파싱한다. args에는 프로그램 이름(argv[0])을 넣지 않는다.
[[nodiscard]] ParseResult parse_server_options(std::span<const std::string_view> args);

// --help와 오류 시 출력할 사용법 문자열
[[nodiscard]] std::string_view server_usage();

}  // namespace wind::server
