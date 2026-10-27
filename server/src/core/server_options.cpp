#include "core/server_options.hpp"

#include "wind/cli.hpp"

#include <limits>
#include <utility>

namespace wind::server {
namespace {

ParseResult fail(std::string message) {
    return ParseResult{.options = std::nullopt, .error = std::move(message)};
}

}  // namespace

ParseResult parse_server_options(std::span<const std::string_view> args) {
    ServerOptions options;

    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];

        if (arg == "--help" || arg == "-h") {
            return ParseResult{.options = std::nullopt, .error = {}, .show_help = true};
        }

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
