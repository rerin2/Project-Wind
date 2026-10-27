#include "wind/cli.hpp"

#include <charconv>

namespace wind::cli {

std::vector<std::string_view> to_args(int argc, char** argv) {
    std::vector<std::string_view> args;
    // argc가 음수일 가능성까지 막아 reserve에 잘못된 크기가 들어가지 않게 한다.
    args.reserve(argc > 0 ? static_cast<std::size_t>(argc) : 0);
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    return args;
}

std::optional<std::uint64_t> parse_uint(std::string_view text, std::uint64_t min,
                                        std::uint64_t max) {
    if (text.empty()) {
        return std::nullopt;
    }
    // from_chars는 로캘 영향을 받지 않고, 공백과 '+' 부호를 허용하지 않는다.
    std::uint64_t value = 0;
    const char* end = text.data() + text.size();
    const auto [ptr, ec] = std::from_chars(text.data(), end, value);
    // 변환 실패, 오버플로, 숫자 뒤 잔여 문자, 범위 초과를 모두 거부한다.
    if (ec != std::errc{} || ptr != end || value < min || value > max) {
        return std::nullopt;
    }
    return value;
}

}  // namespace wind::cli
