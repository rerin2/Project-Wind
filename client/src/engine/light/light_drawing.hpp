#pragma once

// 이 파일은 무엇을 하나요?
// 빛 목록을 보고, 화면에 보이는 빛만 골라 "그리기 명령"으로 바꾸는 도우미예요.
// 화면 밖 빛은 건너뛰고, 한 화면에 그릴 빛 수에 상한을 둬요 (너무 많으면 느려지니까요).

#include "engine/camera/camera2d.hpp"
#include "engine/light/light.hpp"
#include "engine/light/lighting_frame.hpp"

#include <cstddef>
#include <vector>

namespace wind::engine {

inline constexpr std::size_t kMaxLightsPerFrame = 64;

// lights를 frame.lights에 덧붙여요. texture: 가운데가 밝은 둥근 빛 그림 (흰색)
LightStats queue_lights(const std::vector<Light>& lights, const Camera2D& camera,
                        double time_seconds, TextureHandle texture, LightingFrame& frame);

}  // namespace wind::engine
