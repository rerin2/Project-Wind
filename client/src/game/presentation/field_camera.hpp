#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 사진사(카메라)예요. (명세 6.8)
// - 주인공 몸 가운데(발에서 반 칸 위)를 부드럽게 따라가요.
// - 가는 쪽을 조금 더 보여 줘요(앞보기). 빨리 갈수록 더 앞을 봐요.
// - 맵 밖은 보여 주지 않아요.
// 보여 주기 전용이라 게임 결과를 바꾸지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/camera/look_ahead.hpp"
#include "engine/core/math.hpp"

namespace wind::game {

class FieldCamera {
public:
    // viewport: 화면 크기, map_size: 맵 크기(픽셀), tile_size: 칸 크기, start_feet: 처음 볼 발 위치
    FieldCamera(engine::Vec2 viewport, engine::Vec2 map_size, float tile_size, engine::Vec2 start_feet);

    // 화면 한 장마다: velocity로 앞보기를 정하고, speed / max_speed만큼 더 앞을 봐요.
    void follow(engine::Vec2 feet, engine::Vec2 velocity, float speed, float max_speed, float dt_seconds);

    [[nodiscard]] const engine::Camera2D& camera() const { return camera_; }

private:
    // 발 위치 -> 카메라가 볼 몸 가운데
    [[nodiscard]] engine::Vec2 body_center(engine::Vec2 feet) const;

    engine::Camera2D camera_;
    engine::LookAhead look_ahead_;
    float half_tile_;
};

}  // namespace wind::game
