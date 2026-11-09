// 이 파일은 무엇을 하나요?
// 오류 공책(ErrorList)에 한 줄 적기와, 오류를 글 한 줄로 바꾸는 방법이에요.

#include "wind/content/error_list.hpp"

#include "wind/log.hpp"

namespace wind::content {

// 칸 주소가 없으면 "파일: 이유", 있으면 "파일: 칸 주소: 이유"
std::string to_string(const ContentError& error) {
    if (error.field.empty()) {
        return error.file + ": " + error.message;
    }
    return error.file + ": " + error.field + ": " + error.message;
}

void log_content_errors(std::string_view what, const std::vector<ContentError>& errors) {
    log::error("Failed to load {} ({} error(s)):", what, errors.size());
    for (const auto& error : errors) {
        log::error("  {}", to_string(error));
    }
}

void ErrorList::add(std::string file, std::string field, std::string message) {
    if (full()) {
        return;  // 20개가 넘으면 더 적지 않아요 (앞의 오류들로 원인을 충분히 알 수 있어요)
    }
    errors_.push_back({std::move(file), std::move(field), std::move(message)});
}

}  // namespace wind::content
