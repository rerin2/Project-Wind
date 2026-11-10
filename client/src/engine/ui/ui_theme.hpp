#pragma once

// 이 파일은 무엇을 하나요?
// UI 꾸미기 정보(테마)예요. (명세 6.9)
// 창 테두리 그림, 버튼 그림, 글꼴, 글자 색처럼 "UI가 어떻게 보일지"를 한곳에 모아요.
// 값은 content/ui/theme.json에 있고, ui_theme_loader가 읽어요. 코드에는 숫자를 흩어 두지 않아요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

#include <string>

namespace wind::engine {

// 9조각 그림 하나: 그림 판의 어느 부분(source)이고, 테두리 두께(border)가 몇 픽셀인지
struct UiFrame {
    RectI source;
    std::int32_t border = 0;
};

struct UiFont {
    std::string file;      // 글꼴 파일 위치 (콘텐츠 폴더 기준)
    float pixel_size = 12.0f;
};

struct UiTheme {
    std::string image;  // 테두리 그림 판 (콘텐츠 폴더 기준)
    UiFont regular;
    UiFont bold;
    UiFrame panel;           // 창
    UiFrame button;          // 버튼 (보통)
    UiFrame button_focused;  // 버튼 (고른 것)
    Color text{242, 239, 230, 255};          // 보통 글자
    Color text_dim{154, 163, 181, 255};      // 흐린 글자 (설명)
    Color text_focused{255, 233, 168, 255};  // 고른 버튼 글자
    Color text_shadow{16, 19, 28, 255};      // 글자 그림자 (1픽셀 아래)
    std::uint8_t panel_alpha = 235;          // 창 불투명도 (뒤가 살짝 비쳐요)
};

}  // namespace wind::engine
