#include "app/client_options.hpp"

#include "wind/cli.hpp"

#include <limits>
#include <utility>

namespace wind::client {
namespace {

// 창 크기 허용 범위 (최소 320 ~ 최대 8K 너비)
constexpr std::uint64_t kMinWindowSize = 320;
constexpr std::uint64_t kMaxWindowSize = 7680;

// 오류 메시지만 담은 실패 결과를 만든다.
ClientParseResult fail(std::string message) {
    return ClientParseResult{.options = std::nullopt, .error = std::move(message)};
}

}  // namespace

ClientParseResult parse_client_options(std::span<const std::string_view> args) {
    ClientOptions options;  // 기본값에서 시작해 인자로 덮어쓴다

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];

        // --help는 값이 없는 유일한 옵션이라 먼저 처리한다.
        if (arg == "--help" || arg == "-h") {
            return ClientParseResult{.options = std::nullopt, .error = {}, .show_help = true};
        }

        // 나머지 옵션은 모두 "--이름 값" 형태다. 값이 없으면 오류.
        if (i + 1 >= args.size()) {
            return fail("missing value for argument: " + std::string(arg));
        }
        const std::string_view value = args[++i];

        if (arg == "--width" || arg == "--height") {
            const auto parsed = cli::parse_uint(value, kMinWindowSize, kMaxWindowSize);
            if (!parsed) {
                return fail(std::string(arg) + " must be an integer between " +
                            std::to_string(kMinWindowSize) + " and " +
                            std::to_string(kMaxWindowSize));
            }
            // 인자 이름에 따라 너비 또는 높이에 저장한다.
            (arg == "--width" ? options.window_width : options.window_height) =
                static_cast<std::uint32_t>(*parsed);
        } else if (arg == "--max-frames") {
            // 0프레임은 의미가 없으므로 1 이상만 허용한다.
            const auto parsed =
                cli::parse_uint(value, 1, std::numeric_limits<std::uint64_t>::max());
            if (!parsed) {
                return fail("--max-frames must be a positive integer");
            }
            options.max_frames = *parsed;
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

    return ClientParseResult{.options = options};
}

std::string_view client_usage() {
    return "Usage: wind_client [options]\n"
           "  --width <px>         window width, 320-7680 (default 1280)\n"
           "  --height <px>        window height, 320-7680 (default 720)\n"
           "  --max-frames <n>     close after n frames (for smoke tests)\n"
           "  --log-level <level>  trace|debug|info|warn|error|critical|off (default info)\n"
           "  --help               show this message\n";
}

}  // namespace wind::client
