#pragma once

// 이 파일은 무엇을 하나요?
// 색 하나를 담는 작은 메모지예요. 빨강(r)·초록(g)·파랑(b)·진하기(a)를 0~255로 적어요.
// 자료 파일(맵의 빛 색 등)을 읽을 때 써요. 그리기용 색(engine::Color)과 값은 같아요.

#include <cstdint>

namespace wind::world {

struct Rgba {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    friend bool operator==(const Rgba&, const Rgba&) = default;
};

}  // namespace wind::world
