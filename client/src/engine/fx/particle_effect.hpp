#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 효과 = "어떤 점을, 어떻게, 몇 개 만들지" 적은 요리법이에요. (명세 6.6, 14.3)
// 예: 꽃잎 효과 = 분홍 네모 점, 6~9초 동안, 천천히 비스듬히 떨어지며 살랑살랑
// 요리법은 content/fx/*.json 파일에 적고, 게임은 그걸 읽어서 점을 만들어요.
// 그래서 코드를 고치지 않고도 효과 모양을 바꿀 수 있어요.

#include "engine/fx/particle.hpp"
#include "engine/render/render_types.hpp"

#include <cstdint>
#include <string>

namespace wind::engine {

// 최소값~최대값 사이에서 무작위로 고르는 숫자 범위
struct FloatRange {
    float min = 0.0f;
    float max = 0.0f;
};

struct ParticleEffect {
    std::string id;
    ParticleShape shape = ParticleShape::Soft;
    BlendMode blend = BlendMode::Normal;

    FloatRange lifetime{1.0f, 1.0f};  // 수명 (초)
    FloatRange speed{0.0f, 0.0f};     // 처음 빠르기 (픽셀/초)
    float direction = 0.0f;           // 나가는 방향 (도). 0 = 오른쪽, 90 = 아래, 270 = 위
    float spread = 0.0f;              // 방향이 흩어지는 폭 (도). 360이면 사방으로
    float gravity = 0.0f;             // 아래로 당기는 힘 (픽셀/초²)
    float drag = 0.0f;                // 공기 저항
    float spawn_radius = 0.0f;        // 생기는 자리가 중심에서 최대 몇 픽셀 흩어질지

    FloatRange start_size{4.0f, 4.0f};  // 태어날 때 크기 (픽셀)
    float end_size = 0.0f;              // 사라질 때 크기
    Color start_color{255, 255, 255, 255};
    Color end_color{255, 255, 255, 0};
    float fade_in = 0.0f;               // 처음 몇 %(0~1) 동안 서서히 나타날지
    float fade_out = 0.0f;              // 마지막 몇 %(0~1) 동안 서서히 사라질지

    float wobble_amplitude = 0.0f;  // 좌우 살랑임 폭 (픽셀)
    float wobble_frequency = 0.0f;  // 살랑임 빠르기 (1초에 몇 번)

    std::uint32_t burst = 0;  // 한 번에 "팍" 만들 점 개수 (구르기 먼지 등)
    float rate = 0.0f;        // 계속 뿜을 때 1초에 몇 개 (주변 꽃잎 등)
};

}  // namespace wind::engine
