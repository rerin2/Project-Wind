#pragma once

#include "wind/log.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace wind::server {

struct ServerOptions {
    // Initial authoritative tick rate (docs/DECISIONS/0004-server-tick-rate.md).
    std::uint32_t tick_rate_hz = 20;
    // Stop after this many ticks; unset means run until a stop is requested.
    std::optional<std::uint64_t> max_ticks;
    log::Level log_level = log::Level::Info;
};

inline constexpr std::uint32_t kMinTickRateHz = 1;
inline constexpr std::uint32_t kMaxTickRateHz = 120;

struct ParseResult {
    std::optional<ServerOptions> options;  // set on success
    std::string error;                     // set on failure
    bool show_help = false;
};

// Parses command-line arguments, excluding the program name.
[[nodiscard]] ParseResult parse_server_options(std::span<const std::string_view> args);

[[nodiscard]] std::string_view server_usage();

}  // namespace wind::server
