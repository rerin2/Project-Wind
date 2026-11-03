#pragma once

// 이 파일은 무엇을 하나요?
// 게임 화면의 "카메라"예요. 맵은 아주 큰 그림이고, 화면은 그중 일부만 보여 주는 창문이에요.
// 카메라는 창문을 어디에 둘지 정하고, 캐릭터를 따라 창문을 옮겨요.
// - 맵 위 위치(월드 좌표) <-> 화면 위 위치(화면 좌표)를 바꿔 줘요.
// - 화면 위치는 항상 정수 픽셀로 맞춰요. 그래야 도트 그림(픽셀 아트)이 덜덜 떨리지 않아요. (명세 5.4)

#include "engine/core/math.hpp"

#include <optional>

namespace wind::engine {

class Camera2D {
public:
    // 화면(창문)의 크기를 정해요. 우리 게임은 640x360 크기의 화면을 써요.
    void set_viewport(Vec2 size);
    [[nodiscard]] Vec2 viewport() const { return viewport_; }

    // 카메라가 보여 줄 수 있는 맵의 범위 (보통 맵 전체).
    // 이걸 정하면 맵 바깥의 검은 공간은 보이지 않게 카메라가 멈춰요.
    void set_world_bounds(RectF bounds);

    // 어떤 지점을 바로 화면 한가운데에 둬요. 맵에 처음 들어갈 때 써요.
    void center_on(Vec2 world_point);

    // 목표(보통 캐릭터) 쪽으로 카메라를 부드럽게 옮겨요.
    // stiffness가 클수록 빨리 따라가요. 고무줄이 팽팽할수록 빨리 끌려오는 것과 같아요.
    void follow(Vec2 target, float dt_seconds, float stiffness);

    // 카메라가 보고 있는 한가운데 위치
    [[nodiscard]] Vec2 center() const { return center_; }
    // 화면 왼쪽 위 모서리가 맵의 어디를 가리키는지 (정수 픽셀로 반올림)
    [[nodiscard]] Vec2 top_left() const;
    // 화면에 보이는 맵 영역 (네모)
    [[nodiscard]] RectF view_rect() const;

    // 맵 위 위치 -> 화면 위 위치
    [[nodiscard]] Vec2 world_to_screen(Vec2 world) const;
    // 화면 위 위치 -> 맵 위 위치 (나중에 마우스 클릭 위치를 알 때 써요)
    [[nodiscard]] Vec2 screen_to_world(Vec2 screen) const;

private:
    // 카메라 한가운데가 맵 범위를 벗어나면 안쪽으로 되돌려요.
    void clamp_to_bounds();

    Vec2 center_;                  // 카메라가 보는 한가운데
    Vec2 viewport_;                // 화면 크기
    std::optional<RectF> bounds_;  // 맵 범위 (없을 수도 있어요)
};

}  // namespace wind::engine
