// 이 파일은 무엇을 하나요?
// 실행 옵션 값 도우미: 안전한 id 검사, 능력치 "이름=값" 읽기.

#include "app/option_values.hpp"

#include "wind/cli.hpp"

namespace wind::client {

bool is_simple_id(std::string_view id, std::size_t max_length) {
    if (id.empty() || id.size() > max_length) return false;
    for (const char c : id) {
        // 글자 하나씩: 소문자, 숫자, 밑줄만 (점·빗금이 없으니 콘텐츠 폴더 밖으로 못 나가요)
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return false;
    }
    return true;
}

bool parse_stat(std::string_view text, progression::Stats& stats) {
    const auto eq = text.find('=');
    if (eq == std::string_view::npos) return false;
    const auto name = text.substr(0, eq);                     // "intelligence"
    const auto value = cli::parse_uint(text.substr(eq + 1),   // "20"
                                       progression::kStatMin, progression::kStatMax);
    if (!value) return false;
    for (const auto id : progression::kAllStats) {
        if (progression::stat_key(id) == name) {
            stats[id] = static_cast<int>(*value);
            return true;
        }
    }
    return false;
}

}  // namespace wind::client
