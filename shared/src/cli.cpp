#include "wind/cli.hpp"

#include <charconv>

namespace wind::cli {

std::vector<std::string_view> to_args(int argc, char** argv) {
    std::vector<std::string_view> args;
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
    std::uint64_t value = 0;
    const char* end = text.data() + text.size();
    const auto [ptr, ec] = std::from_chars(text.data(), end, value);
    if (ec != std::errc{} || ptr != end || value < min || value > max) {
        return std::nullopt;
    }
    return value;
}

}  // namespace wind::cli
