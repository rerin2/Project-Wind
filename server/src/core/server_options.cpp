#include "core/server_options.hpp"

#include "wind/cli.hpp"

#include <limits>
#include <utility>

namespace wind::server {
namespace {

// 오류 메시지만 담은 실패 결과를 만든다.
ParseResult fail(std::string message) {
    return ParseResult{.options = std::nullopt, .error = std::move(message)};
}

}  // namespace

ParseResult parse_server_options(std::span<const std::string_view> args) {
    ServerOptions options;  // 기본값에서 시작해 인자로 덮어쓴다

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];

        // --help는 값이 없는 유일한 옵션이라 먼저 처리한다.
        if (arg == "--help" || arg == "-h") {
            return ParseResult{.options = std::nullopt, .error = {}, .show_help = true};
        }

        // 나머지 옵션은 모두 "--이름 값" 형태다. 값이 없으면 오류.
        if (i + 1 >= args.size()) {
            return fail("missing value for argument: " + std::string(arg));
        }
        const std::string_view value = args[++i];

        if (arg == "--tick-rate") {
            const auto parsed = cli::parse_uint(value, kMinTickRateHz, kMaxTickRateHz);
            if (!parsed) {
                return fail("--tick-rate must be an integer between " +
                            std::to_string(kMinTickRateHz) + " and " +
                            std::to_string(kMaxTickRateHz));
            }
            options.tick_rate_hz = static_cast<std::uint32_t>(*parsed);
        } else if (arg == "--max-ticks") {
            // 0틱은 의미가 없으므로 1 이상만 허용한다.
            const auto parsed =
                cli::parse_uint(value, 1, std::numeric_limits<std::uint64_t>::max());
            if (!parsed) {
                return fail("--max-ticks must be a positive integer");
            }
            options.max_ticks = *parsed;
        } else if (arg == "--log-level") {
            const auto parsed = log::parse_level(value);
            if (!parsed) {
                return fail("--log-level must be one of trace|debug|info|warn|error|critical|off");
            }
            options.log_level = *parsed;
        } else {
            // 알 수 없는 인자는 무시하지 않고 거부한다 (오타 조기 발견).
            return fail("unknown argument: " + std::string(arg));
        }
    }

    return ParseResult{.options = options};
}

std::string_view server_usage() {
    return "Usage: wind_server [options]\n"
           "  --tick-rate <hz>     world tick rate, 1-120 (default 20)\n"
           "  --max-ticks <n>      stop after n ticks (for smoke tests)\n"
           "  --log-level <level>  trace|debug|info|warn|error|critical|off (default info)\n"
           "  --help               show this message\n";
}

}  // namespace wind::server
