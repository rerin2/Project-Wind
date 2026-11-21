// 이 파일은 무엇을 하나요?
// RGBA 픽셀을 밝기(회색)와 섞어서 흑백 쪽으로 바꾸는 곳이에요.

#include "engine/render/desaturate.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {
namespace {

// 밝기 무게 (합이 256이라 >> 8로 나눌 수 있어요): 빨강 77, 초록 150, 파랑 29 ≈ 0.30 / 0.59 / 0.11
constexpr int kRedWeight = 77;
constexpr int kGreenWeight = 150;
constexpr int kBlueWeight = 29;
// 섞는 비율을 0~256 정수로 바꿔 계산해요 (실수보다 빠르고, 어느 컴퓨터에서나 같은 값)
constexpr int kMixOne = 256;

// 원래 값 c를 회색 g 쪽으로 mix/256만큼 옮겨요
std::uint8_t toward(std::uint8_t c, int gray, int mix) {
    const int value = c + ((gray - c) * mix) / kMixOne;
    return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
}

}  // namespace

void desaturate_rgba(std::span<std::uint8_t> pixels, float amount) {
    if (!std::isfinite(amount) || amount <= 0.0f) {
        return;  // 바꿀 것이 없어요
    }
    const int mix = static_cast<int>(std::lround(std::min(amount, 1.0f) * kMixOne));  // 0~256
    const std::size_t count = pixels.size() / 4;  // 끝에 남는 3바이트 이하는 픽셀이 아니라서 건드리지 않아요
    for (std::size_t i = 0; i < count; ++i) {
        std::uint8_t* p = pixels.data() + i * 4;  // p[0]=R, p[1]=G, p[2]=B, p[3]=A
        const int gray = (p[0] * kRedWeight + p[1] * kGreenWeight + p[2] * kBlueWeight) >> 8;  // 밝기 0~255
        p[0] = toward(p[0], gray, mix);
        p[1] = toward(p[1], gray, mix);
        p[2] = toward(p[2], gray, mix);
        // p[3](알파)는 그대로
    }
}

}  // namespace wind::engine
