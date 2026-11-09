#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 = 먼지, 꽃잎, 반딧불처럼 잠깐 생겼다 사라지는 작은 점 하나예요. (명세 6.6)
// 이 파일은 점 하나가 가진 정보(위치, 속도, 남은 시간, 색...)만 적어 둬요.
// 화면 연출용이라서 게임 규칙(충돌, 피해)에는 아무 영향도 주지 않아요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

#include <cstdint>

namespace wind::engine {

// 점의 모양
// - Soft : 가운데가 진하고 가장자리가 흐린 동그라미 (먼지, 반딧불 빛)
// - Pixel: 또렷한 네모 픽셀 (꽃잎, 불티)
enum class ParticleShape : std::uint8_t { Soft, Pixel };

struct Particle {
    Vec2 position;           // 맵 위 위치 (픽셀)
    Vec2 velocity;           // 1초에 얼마나 움직이는지 (픽셀/초)
    float drag = 0.0f;       // 공기 저항: 클수록 빨리 느려져요
    float gravity = 0.0f;    // 아래로 끌어당기는 힘 (픽셀/초²), 음수면 위로 떠올라요
    float lifetime = 1.0f;   // 태어나서 사라질 때까지 몇 초
    float age = 0.0f;        // 태어난 지 몇 초 됐는지
    float start_size = 4.0f; // 태어날 때 크기 (픽셀)
    float end_size = 0.0f;   // 사라질 때 크기
    Color start_color{255, 255, 255, 255};  // 태어날 때 색 (a = 진하기)
    Color end_color{255, 255, 255, 0};      // 사라질 때 색
    float fade_in = 0.0f;    // 수명 중 처음 몇 %(0~1) 동안 서서히 나타날지 (갑자기 "뿅" 안 나타나게)
    float fade_out = 0.0f;   // 수명 중 마지막 몇 %(0~1) 동안 서서히 사라질지
    float wobble_amplitude = 0.0f;  // 좌우로 살랑이는 폭 (픽셀)
    float wobble_frequency = 0.0f;  // 1초에 몇 번 살랑이는지
    float wobble_phase = 0.0f;      // 살랑임 시작 위치 (점마다 달라서 다 같이 흔들리지 않아요)
    ParticleShape shape = ParticleShape::Soft;
    BlendMode blend = BlendMode::Normal;

    // 수명의 몇 %를 살았는지 (0 = 막 태어남, 1 = 사라질 때)
    [[nodiscard]] float progress() const { return lifetime > 0.0f ? age / lifetime : 1.0f; }
};

}  // namespace wind::engine
