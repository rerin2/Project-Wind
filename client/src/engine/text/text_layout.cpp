// 이 파일은 무엇을 하나요?
// 글을 줄로 나누고(줄바꿈), 줄 수가 넘치면 자르고, 정렬 위치를 계산해요.
// 순서: '\n'으로 문단 나누기 -> 문단마다 띄어쓰기 단위로 줄 채우기 -> 넘치면 자르기 -> 정렬

#include "engine/text/text_layout.hpp"

#include "engine/text/utf8.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {
namespace {

// 띄어쓰기 없이 상자보다 긴 말을 글자 단위로 끊어서 lines에 넣어요.
// 마지막 조각은 다음 말과 이어질 수 있게 돌려줘요.
std::string break_long_word(std::string_view word, const MeasureText& measure, float max_width,
                            std::vector<std::string>& lines) {
    std::string piece;
    for (std::size_t pos = 0; pos < word.size();) {
        const std::size_t length = utf8_char_length(word, pos);
        const std::string_view glyph = word.substr(pos, length);
        if (!piece.empty() && measure(piece + std::string(glyph)) > max_width) {
            lines.push_back(std::move(piece));
            piece.clear();
        }
        piece += glyph;  // 한 글자도 상자보다 넓으면 그 글자 하나만 한 줄이 돼요
        pos += length;
    }
    return piece;
}

// 문단 하나를 상자 폭에 맞게 여러 줄로 나눠요.
void wrap_paragraph(std::string_view paragraph, const MeasureText& measure, float max_width,
                    std::vector<std::string>& lines) {
    if (max_width <= 0.0f) {
        lines.emplace_back(paragraph);
        return;
    }
    std::string line;
    std::size_t start = 0;
    while (start <= paragraph.size()) {
        const std::size_t space = paragraph.find(' ', start);
        const std::size_t end = space == std::string_view::npos ? paragraph.size() : space;
        const std::string_view word = paragraph.substr(start, end - start);

        const std::string candidate = line.empty() ? std::string(word) : line + " " + std::string(word);
        if (measure(candidate) <= max_width) {
            line = candidate;
        } else {
            if (!line.empty()) {
                lines.push_back(std::move(line));
                line.clear();
            }
            if (measure(word) <= max_width) {
                line = std::string(word);
            } else {
                line = break_long_word(word, measure, max_width, lines);
            }
        }
        if (space == std::string_view::npos) {
            break;
        }
        start = space + 1;
    }
    lines.push_back(std::move(line));
}

// 줄 끝을 "..."으로 바꿔서 상자 폭 안에 들어가게 해요.
std::string with_ellipsis(std::string text, const MeasureText& measure, float max_width) {
    std::string result = text + std::string(kEllipsis);
    while (!text.empty() && max_width > 0.0f && measure(result) > max_width) {
        text.resize(utf8_drop_last(text));
        while (!text.empty() && text.back() == ' ') {
            text.pop_back();  // 끝 띄어쓰기는 지워서 "말 ..." 대신 "말..."
        }
        result = text + std::string(kEllipsis);
    }
    return result;
}

}  // namespace

TextLayout layout_text(std::string_view text, const MeasureText& measure,
                       const TextLayoutOptions& options) {
    // 1) 문단 나누기와 줄바꿈
    std::vector<std::string> raw;
    std::size_t start = 0;
    while (true) {
        const std::size_t newline = text.find('\n', start);
        const std::size_t end = newline == std::string_view::npos ? text.size() : newline;
        wrap_paragraph(text.substr(start, end - start), measure, options.max_width, raw);
        if (newline == std::string_view::npos) {
            break;
        }
        start = newline + 1;
    }

    TextLayout layout;
    // 2) 줄 수가 넘치면 자르기. 상자보다 넓은 한 줄(줄바꿈 없는 상자 등)도 자르기.
    if (options.max_lines > 0 && raw.size() > options.max_lines) {
        raw.resize(options.max_lines);
        raw.back() = with_ellipsis(std::move(raw.back()), measure, options.max_width);
        layout.truncated = true;
    }
    for (auto& line : raw) {
        if (options.max_width > 0.0f && measure(line) > options.max_width) {
            line = with_ellipsis(std::move(line), measure, options.max_width);
            layout.truncated = true;
        }
    }

    // 3) 줄마다 폭을 재고, 정렬 위치를 정해요.
    float widest = 0.0f;
    for (auto& line : raw) {
        const float width = measure(line);
        widest = std::max(widest, width);
        layout.lines.push_back({std::move(line), 0.0f, 0.0f, width});
    }
    layout.width = options.max_width > 0.0f ? options.max_width : widest;
    for (std::size_t i = 0; i < layout.lines.size(); ++i) {
        auto& line = layout.lines[i];
        const float spare = std::max(0.0f, layout.width - line.width);
        switch (options.align) {
            case TextAlign::Left: line.x = 0.0f; break;
            case TextAlign::Center: line.x = std::floor(spare * 0.5f); break;
            case TextAlign::Right: line.x = spare; break;
        }
        line.y = static_cast<float>(i) * options.line_height;
    }
    layout.height = static_cast<float>(layout.lines.size()) * options.line_height;
    return layout;
}

}  // namespace wind::engine
