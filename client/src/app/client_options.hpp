#pragma once

#include "wind/log.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace wind::client {

struct ClientOptions {
    std::uint32_t window_width = 1280;
    std::uint32_t window_height = 720;
    // Close after this many frames; unset means run until the window closes.
    std::optional<std::uint64_t> max_frames;
    log::Level log_level = log::Level::Info;
};

struct ClientParseResult {
    std::optional<ClientOptions> options;  // set on success
    std::string error;                     // set on failure
    bool show_help = false;
};

// Parses command-line arguments, excluding the program name.
[[nodiscard]] ClientParseResult parse_client_options(std::span<const std::string_view> args);

[[nodiscard]] std::string_view client_usage();

}  // namespace wind::client
