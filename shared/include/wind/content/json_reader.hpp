#pragma once

// 이 파일은 무엇을 하나요?
// 게임 자료는 JSON(제이슨: 정해진 양식으로 쓴 설명서)이라는 글 파일로 되어 있어요.
// 이 파일은 설명서의 { ... } 묶음 하나를 칸별로 읽으면서 숙제 검사를 하는 도우미예요.
// 칸이 비었거나, 숫자 자리에 글자가 있거나, 숫자가 너무 크면
// "어느 파일의 어느 칸이 왜 틀렸는지"를 오류 공책(ErrorList)에 적어요 (명세 14.4).

#include "wind/content/error_list.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace wind::content {

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
    // 소수도 되는 숫자 칸 읽기 (예: 0.35초). min ~ max 사이여야 해요. 정수(3)도 받아요.
    [[nodiscard]] std::optional<double> number(std::string_view key, double min, double max);
    // 참/거짓(true/false) 칸 읽기
    [[nodiscard]] std::optional<bool> boolean(std::string_view key);
    // [ ... ] 목록 칸 읽기. 항목이 max_items개보다 많으면 틀린 것으로 봐요.
    [[nodiscard]] const nlohmann::json* array(std::string_view key, std::size_t max_items);
    // { ... } 묶음 칸 읽기
    [[nodiscard]] const nlohmann::json* object(std::string_view key);

    // 칸을 모양 검사 없이 그대로 꺼내요. 숫자일 수도, 목록일 수도 있는 칸에 써요.
    // 없으면 "꼭 있어야 하는 칸이 없어요"라고 적고 nullptr을 돌려줘요.
    [[nodiscard]] const nlohmann::json* raw(std::string_view key) { return find(key); }

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

}  // namespace wind::content
