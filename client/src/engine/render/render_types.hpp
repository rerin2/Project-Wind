#pragma once

// 이 파일은 무엇을 하나요?
// 그림 그리기에 쓰는 기본 재료들을 모아 둔 곳이에요. (명세 5.2)
// 게임 코드는 이 재료만 쓰고, SDL(그림을 실제로 그려 주는 도구 상자)의 재료는 몰라도 돼요.
// 그래서 나중에 그리는 도구를 바꿔도 게임 코드는 그대로 둘 수 있어요.

#include "engine/core/math.hpp"

#include <cstdint>

namespace wind::engine {

// 색깔. 빨강(r)·초록(g)·파랑(b)을 0~255로 섞어요. a는 투명도 (255 = 안 비침, 0 = 완전 투명)
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

// 텍스처(그래픽 카드에 올려 둔 그림, 스티커 종이 같은 것)를 가리키는 번호표.
// 그림 자체가 아니라 "몇 번 그림"이라는 번호만 들고 다녀요. 0번은 "그림 없음"이에요.
struct TextureHandle {
    std::uint32_t id = 0;

    [[nodiscard]] bool valid() const { return id != 0; }
    friend bool operator==(const TextureHandle&, const TextureHandle&) = default;
};

// 그림 뒤집기: 안 뒤집음 / 좌우로 뒤집음 / 위아래로 뒤집음
enum class Flip : std::uint8_t { None, Horizontal, Vertical };

// 섞기 방법: 새 그림을 이미 그려진 그림 위에 어떻게 얹을지 (명세 5.2)
// - Normal  : 보통 스티커처럼 덮어요 (투명한 부분은 비쳐 보임)
// - Add     : 색을 더해서 밝게 만들어요. 빛, 불꽃, 반짝임에 써요. 검은색은 아무 변화가 없어요.
// - Multiply: 색을 곱해서 어둡게 만들어요. 그림자, 밤의 어둠에 써요. 흰색은 아무 변화가 없어요.
enum class BlendMode : std::uint8_t { Normal, Add, Multiply };

// 스프라이트(화면에 붙이는 작은 그림 조각) 하나를 그리는 데 필요한 정보
struct SpriteDraw {
    TextureHandle texture;  // 어느 스티커 종이에서
    RectI source;           // 어느 부분을 오려서 (가로·세로가 0이면 그림 전체)
    RectF dest;             // 화면의 어디에, 얼마나 크게 붙일지
    Flip flip = Flip::None;
    float rotation_degrees = 0.0f;  // 붙일 곳 한가운데를 중심으로 시계 방향 회전 각도
    // 색 곱하기와 투명도. 흰색(255,255,255)이면 원래 색 그대로, (255,0,0)이면 빨간 기운만 남아요.
    // a는 투명도예요 (255 = 안 비침, 0 = 안 보임).
    Color tint{255, 255, 255, 255};
    BlendMode blend = BlendMode::Normal;
};

// 그리는 순서 단계 (명세 5.5). 숫자가 작을수록 먼저(아래에) 그려져요.
// 종이 위에 그림을 겹쳐 붙이면 나중에 붙인 게 위에 보이는 것과 같아요.
// YSorted 단계에서는 나무 밑동 같은 낮은 물건과 캐릭터를 "발 위치(y)" 순서로 섞어서 그려요.
// 화면 아래쪽(앞쪽)에 있는 것이 나중에 그려져서 앞에 보여요.
enum class DrawLayer : std::uint8_t {
    Ground,        // 바닥
    GroundDecor,   // 바닥 장식 (꽃 등)
    Shadows,       // 캐릭터 발밑 그림자 (바닥 위, 캐릭터 아래)
    YSorted,       // 낮은 물건 + 캐릭터
    HighObjects,   // 높은 물건 (나뭇잎 등, 캐릭터를 덮음)
    Foreground,    // 맨 앞
    DebugOverlay,  // 개발자용 표시 (충돌 칸 등)
};

// 화면 한 장을 그릴 때의 통계 (디버그 화면에 보여 줘요)
struct RenderStats {
    std::uint32_t draw_calls = 0;  // 그래픽 카드에 "이거 그려 줘"라고 부탁한 횟수
    std::uint32_t culled = 0;      // 화면 밖이라서 그리지 않고 건너뛴 수
};

}  // namespace wind::engine
