// 이 파일은 무엇을 하나요?
// 흑백 바꾸기 도우미(desaturate_rgba)를 숙제 검사해요. (12b)
// - 0(또는 이상한 값)이면 그대로, 1이면 R=G=B (밝기 무게 0.30/0.59/0.11), 알파는 그대로
// - 0.5면 원래 색과 회색의 가운데쯤, 흰색·검은색은 그대로
// - 4의 배수가 아닌 끝 바이트는 건드리지 않아요
#include "engine/render/desaturate.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>

namespace {

using wind::engine::desaturate_rgba;
using Pixels = std::array<std::uint8_t, 8>;

TEST(Desaturate, ZeroOrBadAmountKeepsColors) {
    Pixels pixels{200, 40, 10, 255, 0, 120, 250, 128};
    const Pixels before = pixels;
    desaturate_rgba(pixels, 0.0f);
    desaturate_rgba(pixels, -1.0f);
    desaturate_rgba(pixels, std::numeric_limits<float>::quiet_NaN());
    EXPECT_EQ(pixels, before);
}

TEST(Desaturate, FullAmountMakesGrayAndKeepsAlpha) {
    Pixels pixels{200, 40, 10, 255, 0, 120, 250, 128};
    desaturate_rgba(pixels, 1.0f);
    // 빨강 쪽: (200×77 + 40×150 + 10×29) >> 8 = 84
    EXPECT_EQ(pixels[0], 84);
    EXPECT_EQ(pixels[1], 84);
    EXPECT_EQ(pixels[2], 84);
    EXPECT_EQ(pixels[3], 255);  // 알파 그대로
    // 파랑 쪽: (0×77 + 120×150 + 250×29) >> 8 = 98
    EXPECT_EQ(pixels[4], 98);
    EXPECT_EQ(pixels[5], 98);
    EXPECT_EQ(pixels[6], 98);
    EXPECT_EQ(pixels[7], 128);
}

TEST(Desaturate, HalfAmountIsBetweenAndWhiteBlackStay) {
    Pixels pixels{200, 40, 10, 255, 255, 255, 255, 255};
    desaturate_rgba(pixels, 0.5f);
    EXPECT_EQ(pixels[0], 142);  // 200 -> 84 쪽으로 절반: 200 + (84-200)/2 = 142
    EXPECT_EQ(pixels[1], 62);   // 40 + 44/2
    EXPECT_EQ(pixels[2], 47);   // 10 + 74/2
    EXPECT_EQ(pixels[4], 255);  // 흰색은 흑백이어도 흰색
    EXPECT_EQ(pixels[5], 255);
    EXPECT_EQ(pixels[6], 255);
    std::array<std::uint8_t, 4> black{0, 0, 0, 255};
    desaturate_rgba(black, 1.0f);
    EXPECT_EQ(black[0], 0);
}

TEST(Desaturate, LeavesTrailingBytesAlone) {
    std::array<std::uint8_t, 6> bytes{200, 40, 10, 255, 77, 99};  // 픽셀 1개 + 남는 2바이트
    desaturate_rgba(bytes, 1.0f);
    EXPECT_EQ(bytes[0], 84);
    EXPECT_EQ(bytes[4], 77);
    EXPECT_EQ(bytes[5], 99);
}

TEST(Desaturate, AmountAboveOneActsLikeOne) {
    Pixels a{200, 40, 10, 255, 0, 120, 250, 128};
    Pixels b = a;
    desaturate_rgba(a, 1.0f);
    desaturate_rgba(b, 5.0f);
    EXPECT_EQ(a, b);
}

}  // namespace
