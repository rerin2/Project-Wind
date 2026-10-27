#pragma once

// 클라이언트 실행 옵션과 명령줄 파싱. SDL에 의존하지 않는다.

#include "wind/log.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace wind::client {

// 클라이언트 실행 설정. 기본값만으로도 정상 실행된다.
struct ClientOptions {
    std::uint32_t window_width = 1280;   // 창 너비(픽셀)
    std::uint32_t window_height = 720;   // 창 높이(픽셀)
    // 이 프레임 수만큼 그린 뒤 창을 닫는다. 비어 있으면 사용자가 창을 닫을 때까지 실행한다.
    std::optional<std::uint64_t> max_frames;
    log::Level log_level = log::Level::Info;
};

// 파싱 결과. 성공·실패·도움말 요청 중 하나다.
struct ClientParseResult {
    std::optional<ClientOptions> options;  // 성공 시 설정됨
    std::string error;                     // 실패 시 사람이 읽을 오류 메시지
    bool show_help = false;                // --help 요청 시 true
};

// 명령줄 인자를 파싱한다. args에는 프로그램 이름(argv[0])을 넣지 않는다.
[[nodiscard]] ClientParseResult parse_client_options(std::span<const std::string_view> args);

// --help와 오류 시 출력할 사용법 문자열
[[nodiscard]] std::string_view client_usage();

}  // namespace wind::client
