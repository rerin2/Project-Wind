#pragma once

// 이 파일은 무엇을 하나요?
// 자료 파일(JSON)을 검사하다가 틀린 곳을 찾으면 적어 두는 "오류 공책"이에요. (명세 14.4)
// "어느 파일의 어느 칸이 왜 틀렸는지"를 한 줄씩 적어요.

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace wind::content {

// 틀린 곳 하나에 대한 기록.
// field는 설명서 안에서 틀린 칸의 주소예요. 예: "layers[1].tiles[37]"
// (= layers 목록의 1번 항목 안에 있는 tiles 목록의 37번 칸)
struct ContentError {
    std::string file;
    std::string field;
    std::string message;
};

// 오류 하나를 "파일: 칸 주소: 이유" 모양의 한 줄 글로 바꿔요.
[[nodiscard]] std::string to_string(const ContentError& error);

// 오류를 적는 공책.
// 같은 실수가 수천 번 나와도 공책이 넘치지 않게 20개까지만 적어요.
// 처음 몇 개만 봐도 무엇이 틀렸는지 충분히 알 수 있거든요.
class ErrorList {
public:
    static constexpr std::size_t kMaxErrors = 20;

    void add(std::string file, std::string field, std::string message);
    [[nodiscard]] bool empty() const { return errors_.empty(); }
    [[nodiscard]] bool full() const { return errors_.size() >= kMaxErrors; }
    [[nodiscard]] const std::vector<ContentError>& items() const { return errors_; }
    // 공책 내용을 통째로 넘겨줘요 (복사하지 않고 옮겨요).
    [[nodiscard]] std::vector<ContentError> take() { return std::move(errors_); }

private:
    std::vector<ContentError> errors_;
};

}  // namespace wind::content
