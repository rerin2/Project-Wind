#pragma once

// 이 파일은 무엇을 하나요?
// 글자를 그리는 화가예요. (명세 6.9, 단계 8)
// SDL_ttf(글꼴 도구)로 한글 픽셀 글꼴(Galmuri11)을 읽어서 글을 화면에 써요.
// - 글꼴은 12픽셀 크기에서 픽셀 격자에 딱 맞게 만들어져 있어서, 그 크기로만 써요.
// - 흑백(모노) 힌팅을 켜서 가장자리가 뿌옇지 않고 또렷한 도트 글자가 나와요.
// - 같은 글을 매 화면 새로 만들지 않도록, 만든 글 조각을 보관함(캐시)에 넣어 두고 다시 써요.
// 한 줄만 그려요. 줄바꿈·정렬·잘림은 text_layout이 정해 줘요.
// SDL_ttf를 직접 부르는 곳은 이 파일뿐이에요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"
#include "engine/text/text_layout.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

struct SDL_Renderer;
struct TTF_Font;
struct TTF_Text;
struct TTF_TextEngine;

namespace wind::engine {

// 글꼴 종류
enum class FontStyle : std::uint8_t { Regular, Bold, Count };

class TextRenderer {
public:
    // SDL 붓을 빌려 써요. SDL_ttf는 미리 시작(TTF_Init)되어 있어야 해요.
    explicit TextRenderer(SDL_Renderer* renderer);
    ~TextRenderer();
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    // 글꼴 파일을 읽어요. 실패하면 false와 이유(error). 실패해도 게임은 계속되고, 그 글꼴 글은 안 보여요.
    bool load_font(FontStyle style, const std::filesystem::path& file, float pixel_size,
                   std::string& error);
    [[nodiscard]] bool ready(FontStyle style) const;

    // 한 줄의 폭(픽셀). 글꼴이 없으면 0.
    [[nodiscard]] float measure(FontStyle style, std::string_view text) const;
    // 줄 간격으로 쓸 글꼴 높이(픽셀)
    [[nodiscard]] float line_height(FontStyle style) const;
    // text_layout에 넘겨 줄 "자" (이 글꼴로 폭을 재는 함수)
    [[nodiscard]] MeasureText measurer(FontStyle style) const;

    // 한 줄을 position(왼쪽 위, 정수 픽셀로 맞춤)에 그려요.
    void draw(FontStyle style, Vec2 position, std::string_view text, Color color);

    // 디버그 화면용: 이번 화면에 그린 글 수, 보관함 크기
    [[nodiscard]] std::uint32_t draws_this_frame() const { return draws_; }
    [[nodiscard]] std::size_t cached() const;
    // 화면 한 장을 시작할 때 불러요 (통계 초기화).
    void begin_frame() { draws_ = 0; }

private:
    struct FontSlot {
        TTF_Font* font = nullptr;
        std::unordered_map<std::string, TTF_Text*> cache;  // 글 -> 만들어 둔 글 조각
    };

    TTF_Text* cached_text(FontSlot& slot, std::string_view text);
    static void clear_cache(FontSlot& slot);

    TTF_TextEngine* engine_ = nullptr;
    std::array<FontSlot, static_cast<std::size_t>(FontStyle::Count)> fonts_{};
    std::uint32_t draws_ = 0;
};

}  // namespace wind::engine
