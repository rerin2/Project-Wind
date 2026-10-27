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

// Level 하나에 대응하는 문자열 이름과 spdlog 수준을 묶은 표의 한 줄.
struct LevelName {
    Level level;
    std::string_view name;
    spdlog::level::level_enum backend;
};

// 모든 변환(문자열·spdlog 수준)은 이 표 하나를 기준으로 한다.
constexpr std::array kLevels{
    LevelName{Level::Trace, "trace", spdlog::level::trace},
    LevelName{Level::Debug, "debug", spdlog::level::debug},
    LevelName{Level::Info, "info", spdlog::level::info},
    LevelName{Level::Warn, "warn", spdlog::level::warn},
    LevelName{Level::Error, "error", spdlog::level::err},
    LevelName{Level::Critical, "critical", spdlog::level::critical},
    LevelName{Level::Off, "off", spdlog::level::off},
};

// Level로 표의 항목을 찾는다. 없으면 Info 항목을 돌려준다.
const LevelName& lookup(Level level) {
    const auto it = std::ranges::find(kLevels, level, &LevelName::level);
    return it != kLevels.end() ? *it : kLevels[2];
}

// ASCII 대소문자를 무시하고 두 문자열을 비교한다.
bool iequals(std::string_view a, std::string_view b) {
    return std::ranges::equal(a, b, [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x)) ==
               std::tolower(static_cast<unsigned char>(y));
    });
}

}  // namespace

void init(const Config& config) {
    // 출력 대상: 항상 콘솔(색상), 설정 시 파일 추가
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if (config.file) {
        std::filesystem::create_directories(config.file->parent_path());
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            config.file->string(), /*truncate=*/false));  // 기존 파일 뒤에 이어 쓴다
    }

    auto logger = std::make_shared<spdlog::logger>(config.component, sinks.begin(), sinks.end());
    // 출력 형식 예: 2026-10-27 11:17:00.236 [server] [info] 메시지
    logger->set_pattern("%Y-%m-%d %H:%M:%S.%e [%n] [%^%l%$] %v");
    // 경고 이상은 즉시 기록해 크래시 직전 로그를 잃지 않게 한다.
    logger->flush_on(spdlog::level::warn);
    spdlog::set_default_logger(std::move(logger));
    set_level(config.level);
}

void shutdown() {
    // 버퍼만 비운다. spdlog::shutdown()은 기본 로거를 없애 버려서,
    // 그 뒤의 로그 호출(예: 정적 객체 소멸자)이 null 포인터를 참조하게 된다.
    if (auto logger = spdlog::default_logger()) {
        logger->flush();
    }
}

void set_level(Level level) {
    spdlog::set_level(lookup(level).backend);
}

Level level() {
    // spdlog의 현재 수준을 표에서 역으로 찾아 Level로 바꾼다.
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
