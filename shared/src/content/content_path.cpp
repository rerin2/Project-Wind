// 이 파일은 무엇을 하나요?
// content_path.hpp의 문지기 일을 실제로 해요.
// 파일 위치 글자를 '/' 사이사이의 "구간"(폴더 이름 하나하나)으로 잘라서
// 구간마다 위험하지 않은지 하나씩 검사해요. 숙제 검사처럼 한 글자씩 확인해요.

#include "wind/content/content_path.hpp"

namespace wind::content {
namespace {

// 위치 글자 하나에 허락하는 최대 길이.
// 말도 안 되게 긴 글자로 프로그램을 괴롭히는 일을 막아요.
constexpr std::size_t kMaxPathLength = 260;

}  // namespace

std::optional<std::string> normalize_content_path(std::string_view relative) {
    // 비어 있거나 너무 길면 바로 거절
    if (relative.empty() || relative.size() > kMaxPathLength) {
        return std::nullopt;
    }
    // 맨 앞이 '/'나 '\'이면 "컴퓨터 맨 꼭대기부터"라는 뜻(절대 경로)이라 거절해요.
    if (relative.front() == '/' || relative.front() == '\\') {
        return std::nullopt;
    }

    std::string result;   // 깔끔하게 정리된 결과를 여기에 모아요
    std::string segment;  // 지금 읽고 있는 구간 (폴더 이름 하나)

    // 구간 하나를 다 읽었을 때 검사해서 결과에 붙여요. 위험하면 false.
    // (람다: 함수 안에서 잠깐 쓰는 작은 함수)
    const auto flush = [&]() {
        if (segment.empty() || segment == ".") {
            segment.clear();  // "a//b"의 빈 구간이나 "./"(지금 폴더)는 의미가 없으니 건너뛰어요
            return true;
        }
        if (segment == "..") {
            return false;  // ".."은 "한 칸 위 폴더로"라는 뜻. 상자 밖으로 나갈 수 있으니 금지!
        }
        if (!result.empty()) {
            result += '/';  // 구간 사이에는 '/'를 하나만 넣어요
        }
        result += segment;
        segment.clear();
        return true;
    };

    // 글자를 하나씩 읽어요.
    for (const char c : relative) {
        const auto uc = static_cast<unsigned char>(c);
        if (uc < 0x20 || c == ':') {
            // 0x20보다 작은 글자는 눈에 안 보이는 제어 문자예요.
            // ':'는 "C:" 같은 드라이브 표시나 숨은 파일 표시에 쓰여서 막아요.
            return std::nullopt;
        }
        if (c == '/' || c == '\\') {
            // 구분 기호를 만나면 지금까지 읽은 구간을 검사해요.
            if (!flush()) {
                return std::nullopt;
            }
        } else {
            segment += c;
        }
    }
    // 마지막 구간도 검사해요. 정리했더니 아무것도 안 남으면(예: "./") 거절.
    if (!flush() || result.empty()) {
        return std::nullopt;
    }
    return result;
}

std::optional<std::filesystem::path> resolve_content_path(const std::filesystem::path& root,
                                                          std::string_view relative) {
    const auto normalized = normalize_content_path(relative);
    if (!normalized) {
        return std::nullopt;
    }
    // ".."을 이미 막았으니, 상자(root) 뒤에 붙인 위치는 언제나 상자 안이에요.
    // (바로가기 파일 같은 특별한 경우는 아직 생각하지 않아요.)
    return root / std::filesystem::path(*normalized);
}

}  // namespace wind::content
