// 이 파일은 무엇을 하나요?
// 파일을 안전하게 읽어서 JSON으로 바꾸는 방법을 적은 곳이에요.
// 순서: 크기 확인 -> 파일 열기 -> 글자 읽기 -> JSON으로 바꾸기

#include "wind/content/json_file.hpp"

#include <fstream>
#include <iterator>
#include <system_error>

namespace wind::content {

std::optional<nlohmann::json> parse_json_text(std::string_view text,
                                              const std::string& display_name,
                                              ErrorList& errors) {
    // allow_exceptions=false: JSON 모양이 엉망이어도 프로그램이 멈추지 않고
    // "버려진 값"(discarded)을 돌려줘요. 그걸 보고 오류로 적어요.
    // (예외: 문제가 생겼을 때 프로그램이 갑자기 하던 일을 멈추고 튀어나가는 것)
    auto document = nlohmann::json::parse(text, /*cb=*/nullptr, /*allow_exceptions=*/false);
    if (document.is_discarded()) {
        errors.add(display_name, "", "not valid JSON");
        return std::nullopt;
    }
    return document;
}

std::optional<nlohmann::json> read_json_file(const std::filesystem::path& path,
                                             const std::string& display_name,
                                             ErrorList& errors) {
    // 1) 파일 크기부터 확인해요. 파일이 없으면 여기서 알게 돼요.
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    if (ec) {
        errors.add(display_name, "", "cannot read file (" + path.string() + "): " + ec.message());
        return std::nullopt;
    }
    if (size > kMaxContentFileBytes) {
        errors.add(display_name, "", "file is too large (" + std::to_string(size) + " bytes)");
        return std::nullopt;
    }

    // 2) 파일을 열어 내용을 통째로 글자로 읽어요.
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        errors.add(display_name, "", "cannot open file (" + path.string() + ")");
        return std::nullopt;
    }
    const std::string text{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    // 3) 읽은 글자를 JSON으로 바꿔요.
    return parse_json_text(text, display_name, errors);
}

}  // namespace wind::content
