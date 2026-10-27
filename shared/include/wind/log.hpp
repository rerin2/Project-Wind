#pragma once

// 클라이언트·서버·도구가 공통으로 쓰는 로그 인터페이스.
// 실제 출력은 spdlog가 담당하지만, 호출하는 쪽은 wind::log만 사용한다.
// 그래서 나중에 백엔드를 바꿔도 게임 코드는 고칠 필요가 없다.
// 비밀번호·세션 비밀·토큰은 절대 로그에 남기지 않는다 (docs/SECURITY.md).
//
// 사용 예: wind::log::info("player {} joined", name);

#include <spdlog/spdlog.h>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace wind::log {

// 명세 18.3의 로그 수준. Off는 모든 출력을 끈다.
enum class Level { Trace, Debug, Info, Warn, Error, Critical, Off };

// init()에 넘기는 로그 설정.
struct Config {
    std::string component = "wind";  // 모든 줄에 표시되는 이름 (예: "server")
    Level level = Level::Info;       // 이 수준 미만의 메시지는 버린다
    std::optional<std::filesystem::path> file;  // 지정하면 파일에도 함께 기록
};

// 기본 로거를 새 설정으로 교체한다. 여러 번 호출해도 안전하다.
void init(const Config& config);

// 남은 로그를 모두 출력한다. 프로세스 종료 직전에 호출한다.
void shutdown();

// 실행 중 출력 수준을 바꾸거나 현재 수준을 확인한다.
void set_level(Level level);
[[nodiscard]] Level level();

// "info", "WARN" 같은 문자열 <-> Level 변환 (대소문자 무시).
// 알 수 없는 이름이면 parse_level은 nullopt를 돌려준다.
[[nodiscard]] std::optional<Level> parse_level(std::string_view text);
[[nodiscard]] std::string_view to_string(Level level);

// 수준별 로그 함수. 형식 문자열은 {fmt} 문법이며 컴파일 시점에 검사된다.
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
