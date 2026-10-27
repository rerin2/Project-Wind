#pragma once

// Common logging interface for client, server, and tools.
// spdlog is the backend; callers use only the wind::log namespace so the
// backend can be swapped without touching game code.
// Never log passwords, session secrets, or tokens (see docs/SECURITY.md).

#include <spdlog/spdlog.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace wind::log {

enum class Level { Trace, Debug, Info, Warn, Error, Critical, Off };

struct Config {
    std::string component = "wind";  // shown in every line, e.g. "server"
    Level level = Level::Info;
    std::optional<std::filesystem::path> file;  // also write to this file when set
};

// Replaces the default logger. Safe to call more than once.
void init(const Config& config);

// Flushes all sinks. Call before process exit.
void shutdown();

void set_level(Level level);
[[nodiscard]] Level level();

[[nodiscard]] std::optional<Level> parse_level(std::string_view text);
[[nodiscard]] std::string_view to_string(Level level);

template <typename... Args>
void trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    spdlog::trace(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void debug(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    spdlog::debug(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    spdlog::info(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    spdlog::warn(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    spdlog::error(fmt, std::forward<Args>(args)...);
}
template <typename... Args>
void critical(spdlog::format_string_t<Args...> fmt, Args&&... args) {
    spdlog::critical(fmt, std::forward<Args>(args)...);
}

}  // namespace wind::log
