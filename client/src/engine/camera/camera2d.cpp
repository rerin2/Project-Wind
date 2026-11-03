// 이 파일은 무엇을 하나요?
// 카메라(화면 창문)가 움직이고, 위치를 바꿔 계산하는 방법을 실제로 적은 곳이에요.

#include "engine/camera/camera2d.hpp"

#include <algorithm>
#include <cmath>

namespace wind::engine {
namespace {

// 가로 또는 세로 한 방향에 대해, 카메라 한가운데가 갈 수 있는 곳을 제한해요.
// lo는 맵의 시작, extent는 맵의 길이, view는 화면의 길이예요.
// 맵이 화면보다 작으면 맵을 화면 가운데에 둬요.
float clamp_axis(float center, float view, float lo, float extent) {
    if (extent <= view) {
        return lo + extent * 0.5f;
    }
    // 화면의 절반만큼은 안쪽에 있어야 맵 끝이 화면 끝과 딱 맞아요.
    return std::clamp(center, lo + view * 0.5f, lo + extent - view * 0.5f);
}

}  // namespace

void Camera2D::set_viewport(Vec2 size) {
    viewport_ = size;
    clamp_to_bounds();
}

void Camera2D::set_world_bounds(RectF bounds) {
    bounds_ = bounds;
    clamp_to_bounds();
}

void Camera2D::center_on(Vec2 world_point) {
    center_ = world_point;
    clamp_to_bounds();
}

void Camera2D::follow(Vec2 target, float dt_seconds, float stiffness) {
    // 매번 "남은 거리의 일정 비율"만큼 다가가요. 처음엔 빨리, 가까워질수록 천천히 다가가요.
    // exp(지수 함수)를 쓰면 화면이 빠르게 그려지든 느리게 그려지든 따라가는 느낌이 똑같아요.
    const float t = 1.0f - std::exp(-stiffness * dt_seconds);
    center_ = center_ + (target - center_) * t;
    clamp_to_bounds();
}

Vec2 Camera2D::top_left() const {
    // 한가운데에서 화면 절반만큼 왼쪽 위로 가면 화면 왼쪽 위 모서리예요.
    // round(반올림)로 정수 픽셀에 맞춰서 그림이 흐리거나 떨리지 않게 해요.
    return {std::round(center_.x - viewport_.x * 0.5f), std::round(center_.y - viewport_.y * 0.5f)};
}

RectF Camera2D::view_rect() const {
    const Vec2 origin = top_left();
    return {origin.x, origin.y, viewport_.x, viewport_.y};
}

// 맵 위치에서 화면 왼쪽 위 위치를 빼면 화면 안에서의 위치가 돼요.
Vec2 Camera2D::world_to_screen(Vec2 world) const {
    return world - top_left();
}

// 반대로 화면 위치에 화면 왼쪽 위 위치를 더하면 맵 위치가 돼요.
Vec2 Camera2D::screen_to_world(Vec2 screen) const {
    return screen + top_left();
}

void Camera2D::clamp_to_bounds() {
    if (!bounds_) {
        return;  // 맵 범위를 정하지 않았으면 아무 데나 갈 수 있어요
    }
    center_.x = clamp_axis(center_.x, viewport_.x, bounds_->x, bounds_->w);
    center_.y = clamp_axis(center_.y, viewport_.y, bounds_->y, bounds_->h);
}

}  // namespace wind::engine
