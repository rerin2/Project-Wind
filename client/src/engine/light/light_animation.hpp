#pragma once

// 이 파일은 무엇을 하나요?
// 빛이 시간에 따라 얼마나 밝은지, 얼마나 크게 퍼지는지 계산하는 도우미예요.
// - 깜빡임(flicker): 빠르기가 다른 물결 두 개를 섞어서 불규칙하게 흔들려요 (횃불 느낌)
// - 숨쉬기(pulse): 한 가지 빠르기로 천천히 밝아졌다 어두워져요 (마법 느낌)

#include "engine/light/light.hpp"

namespace wind::engine {

// 지금 밝기 (0~intensity). time_seconds: 게임이 시작된 뒤 흐른 시간
[[nodiscard]] float light_strength(const Light& light, double time_seconds);

// 지금 크기 배율 (깜빡일 때 빛이 살짝 줄었다 커져요). 1이 원래 크기.
[[nodiscard]] float light_radius_scale(const Light& light, double time_seconds);

}  // namespace wind::engine
