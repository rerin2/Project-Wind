#include "wind/log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <vector>

namespace wind::log {
namespace {

struct LevelName {
    Level level;
    std::string_view name;
    spdlog::level::level_enum backend;
};

constexpr std::array kLevels{
    LevelName{Level::Trace, "trace", spdlog::level::trace},
    LevelName{Level::Debug, "debug", spdlog::level::debug},
    LevelName{Level::Info, "info", spdlog::level::info},
    LevelName{Level::Warn, "warn", spdlog::level::warn},
    LevelName{Level::Error, "error", spdlog::level::err},
    LevelName{Level::Critical, "critical", spdlog::level::critical},
    LevelName{Level::Off, "off", spdlog::level::off},
};

const LevelName& lookup(Level level) {
    const auto it = std::ranges::find(kLevels, level, &LevelName::level);
    return it != kLevels.end() ? *it : kLevels[2];
}

bool iequals(std::string_view a, std::string_view b) {
    return std::ranges::equal(a, b, [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x)) ==
               std::tolower(static_cast<unsigned char>(y));
    });
}

}  // namespace

void init(const Config& config) {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if (config.file) {
        std::filesystem::create_directories(config.file->parent_path());
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            config.file->string(), /*truncate=*/false));
    }

    auto logger = std::make_shared<spdlog::logger>(config.component, sinks.begin(), sinks.end());
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%n] [%^%l%$] %v");
    logger->flush_on(spdlog::level::warn);
    spdlog::set_default_logger(std::move(logger));
    set_level(config.level);
}

void shutdown() {
    // Flush only: spdlog::shutdown() drops the default logger, and any later
    // log call (e.g. from a static destructor) would dereference null.
    if (auto logger = spdlog::default_logger()) {
        logger->flush();
    }
}

void set_level(Level level) {
    spdlog::set_level(lookup(level).backend);
}

Level level() {
    const auto backend = spdlog::get_level();
    const auto it = std::ranges::find(kLevels, backend, &LevelName::backend);
    return it != kLevels.end() ? it->level : Level::Info;
}

std::optional<Level> parse_level(std::string_view text) {
    const auto it = std::ranges::find_if(
        kLevels, [text](const LevelName& entry) { return iequals(entry.name, text); });
    if (it == kLevels.end()) {
        return std::nullopt;
    }
    return it->level;
}

std::string_view to_string(Level level) {
    return lookup(level).name;
}

}  // namespace wind::log
