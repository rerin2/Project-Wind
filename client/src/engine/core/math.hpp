#pragma once

// 이 파일은 무엇을 하나요?
// 게임에서 "위치"와 "네모 칸"을 적어 두는 작은 메모지들을 모아 둔 곳이에요.
// - Vec2: 소수점까지 쓰는 위치 (화면이나 맵 위의 픽셀 위치)
// - Vec2i: 정수만 쓰는 위치 (그림 파일 안의 픽셀 위치)
// - RectF / RectI: 왼쪽 위 위치와 가로·세로 크기로 정한 네모

#include <cstdint>

namespace wind::engine {

// 소수점이 있는 위치. 예: 캐릭터가 두 칸 사이를 걸어가는 중이면 x = 3.5 같은 값이 돼요.
struct Vec2 {
    float x = 0.0f;  // 가로 위치 (오른쪽으로 갈수록 커짐)
    float y = 0.0f;  // 세로 위치 (아래로 갈수록 커짐)

    // 위치끼리 더하기·빼기, 숫자 곱하기를 쉽게 쓰려고 만든 계산 규칙이에요.
    friend Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
    friend Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
    friend Vec2 operator*(Vec2 v, float s) { return {v.x * s, v.y * s}; }
    friend bool operator==(const Vec2&, const Vec2&) = default;
};

// 정수만 쓰는 위치. 그림 파일 속 "몇 번째 픽셀"처럼 딱 떨어지는 값에 써요.
struct Vec2i {
    std::int32_t x = 0;
    std::int32_t y = 0;

    friend bool operator==(const Vec2i&, const Vec2i&) = default;
};

// 소수점이 있는 네모. x, y는 왼쪽 위 모서리, w는 가로 길이, h는 세로 길이예요.
struct RectF {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    // 오른쪽 끝과 아래쪽 끝의 위치
    [[nodiscard]] float right() const { return x + w; }
    [[nodiscard]] float bottom() const { return y + h; }
    // 두 네모가 겹치는지 알려 줘요. 모서리끼리 살짝 닿기만 하면 겹친 게 아니에요.
    [[nodiscard]] bool intersects(const RectF& o) const {
        return x < o.right() && o.x < right() && y < o.bottom() && o.y < bottom();
    }
    friend bool operator==(const RectF&, const RectF&) = default;
};

// 정수만 쓰는 네모. 주로 "그림 파일(텍스처) 안에서 어느 부분을 오려 쓸지"를 적어요.
struct RectI {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t w = 0;
    std::int32_t h = 0;

    friend bool operator==(const RectI&, const RectI&) = default;
};

}  // namespace wind::engine
