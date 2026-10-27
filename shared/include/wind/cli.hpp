#pragma once

// Minimal command-line helpers shared by the client, server, and tools.

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace wind::cli {

// Copies argv into views. The views stay valid for the life of argv.
[[nodiscard]] std::vector<std::string_view> to_args(int argc, char** argv);

// Parses a whole base-10 unsigned integer within [min, max].
// Rejects signs, whitespace, trailing characters, and out-of-range values.
[[nodiscard]] std::optional<std::uint64_t> parse_uint(std::string_view text,
                                                      std::uint64_t min,
                                                      std::uint64_t max);

}  // namespace wind::cli
