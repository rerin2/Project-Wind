#pragma once

// 이 파일은 무엇을 하나요?
// 글을 상자 안에 어떻게 놓을지 정해요. (명세 6.9, 13.3)
// - 줄바꿈: 상자 폭을 넘으면 띄어쓰기에서 다음 줄로 넘겨요. 띄어쓰기 없는 아주 긴 말은 글자 단위로 끊어요.
// - 정렬: 왼쪽 / 가운데 / 오른쪽
// - 잘림: 줄 수가 넘치면 마지막 줄 끝을 "..."으로 바꾸고 "잘렸어요"(truncated) 표시를 해요.
//   잘림 표시를 보고 화면 글자가 상자 밖으로 넘치는지 검사할 수 있어요.
// 글자 폭은 measure 함수로 물어봐요. 진짜 글꼴(SDL_ttf)을 몰라도 돼서 창 없이 테스트할 수 있어요.

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::engine {

enum class TextAlign : unsigned char { Left, Center, Right };

// 글 한 줄의 폭(픽셀)을 재는 함수
using MeasureText = std::function<float(std::string_view)>;

struct TextLayoutOptions {
    float max_width = 0.0f;   // 상자 폭. 0이면 줄바꿈 없음 (줄바꿈 문자 '\n'만)
    std::size_t max_lines = 0;  // 최대 줄 수. 0이면 제한 없음
    float line_height = 0.0f;  // 줄 간격 (픽셀)
    TextAlign align = TextAlign::Left;
};

struct TextLine {
    std::string text;
    float x = 0.0f;  // 상자 왼쪽에서 얼마나 떨어져 그릴지 (정렬 반영)
    float y = 0.0f;  // 상자 위에서 얼마나 떨어져 그릴지
    float width = 0.0f;
};

struct TextLayout {
    std::vector<TextLine> lines;
    float width = 0.0f;   // 상자 폭 (max_width가 있으면 그것, 없으면 가장 긴 줄)
    float height = 0.0f;  // 줄 수 × 줄 간격
    bool truncated = false;  // 넘쳐서 잘렸나요?
};

// 줄을 잘랐을 때 끝에 붙이는 글자 (모든 글꼴에 있는 마침표 세 개)
inline constexpr std::string_view kEllipsis = "...";

[[nodiscard]] TextLayout layout_text(std::string_view text, const MeasureText& measure,
                                     const TextLayoutOptions& options);

}  // namespace wind::engine
