#pragma once

// 이 파일은 무엇을 하나요?
// 게임 자료는 JSON(제이슨: 정해진 양식으로 쓴 설명서)이라는 글 파일로 되어 있어요.
// 이 파일은 그 설명서를 읽으면서 숙제 검사를 하는 도우미예요.
// 칸이 비었거나, 숫자 자리에 글자가 있거나, 숫자가 너무 크면
// "어느 파일의 어느 칸이 왜 틀렸는지"를 공책(오류 목록)에 적어 둬요 (명세 16.4).

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
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

// 읽어 오기 결과 상자.
// 성공하면 value에 결과가 들어 있고, 실패하면 value는 비고 errors에 이유가 들어 있어요.
// (template: 어떤 종류의 물건이든 담을 수 있는 상자 모양)
template <typename T>
struct LoadResult {
    std::optional<T> value;
    std::vector<ContentError> errors;

    [[nodiscard]] bool ok() const { return value.has_value(); }
};

// 설명서의 { ... } 묶음(객체) 하나를 칸별로 읽는 도우미.
// 칸이 없거나 모양이 틀리면 공책에 적고 "없음"(nullopt)을 돌려줘요.
class JsonReader {
public:
    // path: 이 묶음이 설명서 안에서 어디에 있는지 (맨 바깥 묶음이면 빈 글자)
    JsonReader(const nlohmann::json& object, std::string file, std::string path, ErrorList& errors);

    // 받은 것이 { ... } 묶음이 아니면 false예요. 그때는 무엇을 읽어도 실패해요.
    [[nodiscard]] bool valid() const { return valid_; }

    // 글자 칸 읽기. 비어 있거나 max_length보다 길면 틀린 것으로 봐요.
    [[nodiscard]] std::optional<std::string> string(std::string_view key,
                                                    std::size_t max_length = 256);
    // 정수 칸 읽기. min ~ max 사이여야 해요.
    [[nodiscard]] std::optional<std::int64_t> integer(std::string_view key, std::int64_t min,
                                                      std::int64_t max);
    // 참/거짓(true/false) 칸 읽기
    [[nodiscard]] std::optional<bool> boolean(std::string_view key);
    // [ ... ] 목록 칸 읽기. 항목이 max_items개보다 많으면 틀린 것으로 봐요.
    [[nodiscard]] const nlohmann::json* array(std::string_view key, std::size_t max_items);
    // { ... } 묶음 칸 읽기
    [[nodiscard]] const nlohmann::json* object(std::string_view key);

    // 그 칸이 있는지만 살짝 확인해요 (없어도 공책에 적지 않아요).
    [[nodiscard]] bool has(std::string_view key) const;

    // 칸 주소 만들기. 예: 이 묶음이 "layers[2]"이면 field_path("name") -> "layers[2].name"
    [[nodiscard]] std::string field_path(std::string_view key) const;
    // 목록 안 항목 주소 만들기. 예: index_path("tiles", 5) -> "tiles[5]"
    [[nodiscard]] std::string index_path(std::string_view key, std::size_t index) const;

    // 공책에 오류를 하나 적어요.
    void error(std::string field, std::string message);
    [[nodiscard]] const std::string& file() const { return file_; }
    [[nodiscard]] ErrorList& errors() { return errors_; }

private:
    // 칸을 찾아요. 없으면 "꼭 있어야 하는 칸이 없어요"라고 적어요.
    const nlohmann::json* find(std::string_view key);

    const nlohmann::json& object_;
    std::string file_;
    std::string path_;
    ErrorList& errors_;
    bool valid_ = true;
};

// 설명서 파일의 최대 크기 (8MB).
// 엄청나게 큰 파일을 읽느라 컴퓨터가 멈추지 않게, 읽기 전에 크기부터 확인해요.
inline constexpr std::uintmax_t kMaxContentFileBytes = 8u * 1024u * 1024u;

// 파일을 읽어 JSON으로 바꿔요. 실패하면 이유를 공책에 적어요.
// display_name: 오류 메시지에 보여 줄 파일 이름 (보통 콘텐츠 폴더 기준 위치)
[[nodiscard]] std::optional<nlohmann::json> read_json_file(const std::filesystem::path& path,
                                                           const std::string& display_name,
                                                           ErrorList& errors);

// 글자를 바로 JSON으로 바꿔요 (테스트나 도구에서 써요).
[[nodiscard]] std::optional<nlohmann::json> parse_json_text(std::string_view text,
                                                            const std::string& display_name,
                                                            ErrorList& errors);

}  // namespace wind::content
