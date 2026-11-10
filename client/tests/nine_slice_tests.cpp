// 이 파일은 무엇을 하나요?
// 9조각 창 테두리 계산을 숙제 검사(테스트)해요.
// - 모서리는 늘어나지 않고, 가운데만 창 크기만큼 늘어나요
// - 창이 너무 작으면 테두리를 줄여서 조각이 겹치지 않아요
#include "engine/ui/nine_slice.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::nine_slice;
using wind::engine::RectF;
using wind::engine::RectI;

// 24x24 그림, 테두리 8 -> 100x50 창
TEST(NineSlice, CornersKeepSizeAndCenterStretches) {
    const auto pieces = nine_slice({0, 0, 24, 24}, 8, {10.0f, 20.0f, 100.0f, 50.0f});
    // 왼쪽 위 모서리: 그림 (0,0,8,8) -> 화면 (10,20,8,8)
    EXPECT_EQ(pieces[0].source, (RectI{0, 0, 8, 8}));
    EXPECT_EQ(pieces[0].dest, (RectF{10.0f, 20.0f, 8.0f, 8.0f}));
    // 가운데: 그림 (8,8,8,8) -> 화면 (18,28,84,34)
    EXPECT_EQ(pieces[4].source, (RectI{8, 8, 8, 8}));
    EXPECT_EQ(pieces[4].dest, (RectF{18.0f, 28.0f, 84.0f, 34.0f}));
    // 오른쪽 아래 모서리: 그림 (16,16,8,8) -> 화면 (102,62,8,8)
    EXPECT_EQ(pieces[8].source, (RectI{16, 16, 8, 8}));
    EXPECT_EQ(pieces[8].dest, (RectF{102.0f, 62.0f, 8.0f, 8.0f}));
    // 위 가장자리는 가로로만 늘어나요
    EXPECT_EQ(pieces[1].dest, (RectF{18.0f, 20.0f, 84.0f, 8.0f}));
}

// 그림 판 안쪽 위치(source.x, y)도 그대로 따라가요.
TEST(NineSlice, UsesSourceOffset) {
    const auto pieces = nine_slice({24, 0, 24, 16}, 5, {0.0f, 0.0f, 60.0f, 16.0f});
    EXPECT_EQ(pieces[0].source, (RectI{24, 0, 5, 5}));
    EXPECT_EQ(pieces[4].source, (RectI{29, 5, 14, 6}));
    EXPECT_EQ(pieces[8].source, (RectI{43, 11, 5, 5}));
}

// 10x6 창에 테두리 8: 테두리를 3(높이 6의 절반)으로 줄여서 가운데 높이가 0이 돼요. 겹치지 않아요.
TEST(NineSlice, SmallWindowShrinksBorder) {
    const auto pieces = nine_slice({0, 0, 24, 24}, 8, {0.0f, 0.0f, 10.0f, 6.0f});
    EXPECT_FLOAT_EQ(pieces[0].dest.w, 3.0f);
    EXPECT_FLOAT_EQ(pieces[4].dest.h, 0.0f);
    EXPECT_FLOAT_EQ(pieces[2].dest.x + pieces[2].dest.w, 10.0f);
}

}  // namespace
