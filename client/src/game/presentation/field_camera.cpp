// 이 파일은 무엇을 하나요?
// 카메라가 주인공을 따라가고 가는 쪽을 조금 더 보여 주는 계산이에요.
// (field_state.cpp에서 그대로 옮겨 왔어요. 동작은 같아요.)

#include "game/presentation/field_camera.hpp"

#include <algorithm>

namespace wind::game {
namespace {

// 카메라가 플레이어를 따라가는 속도 (클수록 빨리 따라붙어요)
constexpr float kCameraStiffness = 8.0f;
// 카메라 앞보기: 최대 몇 픽셀 앞을 보여 줄지, 얼마나 빨리 따라갈지
constexpr float kLookAheadDistance = 28.0f;
constexpr float kLookAheadSharpness = 3.0f;

}  // namespace

// 화면 크기와 맵 경계를 정하고, 처음에는 주인공 몸 가운데를 바로 봐요.
FieldCamera::FieldCamera(engine::Vec2 viewport, engine::Vec2 map_size, float tile_size,
                         engine::Vec2 start_feet)
    : look_ahead_(kLookAheadDistance, kLookAheadSharpness), half_tile_(tile_size * 0.5f) {
    camera_.set_viewport(viewport);
    camera_.set_world_bounds({0.0f, 0.0f, map_size.x, map_size.y});  // 맵 밖은 안 보여요
    camera_.center_on(body_center(start_feet));
    view_ = camera_;  // 처음엔 흔들림 없음
}

void FieldCamera::set_shake(engine::Vec2 offset) {
    view_ = camera_;                                // 따라가는 카메라를 복사해서
    view_.center_on(camera_.center() + offset);     // 흔들림만큼 옮겨요 (맵 끝에서는 경계에 막혀요)
}

void FieldCamera::follow(engine::Vec2 feet, engine::Vec2 velocity, float speed, float max_speed,
                         float dt_seconds) {
    // 빠르기 비율 0~1: 서 있으면 0이라 앞보기가 가운데로 돌아와요
    const float run = max_speed > 0.0f ? std::min(1.0f, speed / max_speed) : 0.0f;
    look_ahead_.update(engine::normalized(velocity) * run, dt_seconds);
    camera_.follow(body_center(feet) + look_ahead_.offset(), dt_seconds, kCameraStiffness);
    view_ = camera_;  // 흔들림은 set_shake가 다시 얹어요
}

engine::Vec2 FieldCamera::body_center(engine::Vec2 feet) const {
    return feet - engine::Vec2{0.0f, half_tile_};
}

}  // namespace wind::game
