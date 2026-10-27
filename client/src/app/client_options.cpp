#include "app/client_options.hpp"

#include "wind/cli.hpp"

#include <limits>
#include <utility>

namespace wind::client {
namespace {

constexpr std::uint64_t kMinWindowSize = 320;
constexpr std::uint64_t kMaxWindowSize = 7680;

ClientParseResult fail(std::string message) {
    return ClientParseResult{.options = std::nullopt, .error = std::move(message)};
}

}  // namespace

ClientParseResult parse_client_options(std::span<const std::string_view> args) {
    ClientOptions options;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];

        if (arg == "--help" || arg == "-h") {
            return ClientParseResult{.options = std::nullopt, .error = {}, .show_help = true};
        }

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
            (arg == "--width" ? options.window_width : options.window_height) =
                static_cast<std::uint32_t>(*parsed);
        } else if (arg == "--max-frames") {
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
