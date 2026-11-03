// 이 파일은 무엇을 하나요?
// 카메라(화면 창문)가 맞게 움직이는지 숙제 검사(테스트)를 해요.
// - 맵 위치 <-> 화면 위치가 맞게 바뀌나
// - 맵 끝에서 멈추나 (맵 밖이 안 보이나)
// - 화면 위치가 정수 픽셀로 맞춰지나
// - 캐릭터를 부드럽게 따라가나
#include "engine/camera/camera2d.hpp"

#include <gtest/gtest.h>

namespace {

using wind::engine::Camera2D;
using wind::engine::Vec2;

// 검사용 카메라: 화면 640x360, 맵 1280x960 픽셀
Camera2D make_camera() {
    Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.set_world_bounds({0.0f, 0.0f, 1280.0f, 960.0f});
    return camera;
}

// 카메라 한가운데에 둔 점은 화면 한가운데 (320, 180)에 보여야 해요.
TEST(Camera2D, CenteredPointMapsToScreenCenter) {
    auto camera = make_camera();
    camera.center_on({500.0f, 400.0f});
    EXPECT_EQ(camera.world_to_screen({500.0f, 400.0f}), (Vec2{320.0f, 180.0f}));
    EXPECT_EQ(camera.top_left(), (Vec2{180.0f, 220.0f}));
}

// 맵 위치 -> 화면 위치 -> 다시 맵 위치로 바꾸면 처음 위치로 돌아와야 해요. (왕복 검사)
TEST(Camera2D, ScreenWorldRoundTrip) {
    auto camera = make_camera();
    camera.center_on({700.0f, 500.0f});
    const Vec2 world{812.0f, 455.0f};
    EXPECT_EQ(camera.screen_to_world(camera.world_to_screen(world)), world);
}

// 맵 모서리를 보라고 해도, 카메라는 맵 밖이 보이지 않는 곳에서 멈춰야 해요.
TEST(Camera2D, ClampsToWorldBounds) {
    auto camera = make_camera();
    camera.center_on({0.0f, 0.0f});
    EXPECT_EQ(camera.top_left(), (Vec2{0.0f, 0.0f}));
    camera.center_on({5000.0f, 5000.0f});
    EXPECT_EQ(camera.top_left(), (Vec2{640.0f, 600.0f}));
}

// 맵이 화면보다 작으면, 맵을 화면 한가운데에 놓아야 해요. (액자 가운데 사진처럼)
TEST(Camera2D, CentersSmallWorld) {
    Camera2D camera;
    camera.set_viewport({640.0f, 360.0f});
    camera.set_world_bounds({0.0f, 0.0f, 320.0f, 160.0f});
    camera.center_on({0.0f, 0.0f});
    EXPECT_EQ(camera.top_left(), (Vec2{-160.0f, -100.0f}));
}

// 카메라가 500.4처럼 소수점 위치에 있어도, 화면 왼쪽 위는 정수(180, 221)로 반올림돼야 해요.
// 그래야 도트 그림이 흐릿하게 떨리지 않아요.
TEST(Camera2D, TopLeftIsPixelSnapped) {
    auto camera = make_camera();
    camera.center_on({500.4f, 400.6f});
    const Vec2 origin = camera.top_left();
    EXPECT_FLOAT_EQ(origin.x, 180.0f);
    EXPECT_FLOAT_EQ(origin.y, 221.0f);
}

// 따라가기: 한 번에 확 가지 않고 조금씩 다가가야 하고, 오래 지나면 결국 도착해야 해요.
// (600번 = 10초 동안 따라가기)
TEST(Camera2D, FollowApproachesTarget) {
    auto camera = make_camera();
    camera.center_on({400.0f, 400.0f});
    camera.follow({600.0f, 400.0f}, 1.0f / 60.0f, 10.0f);
    EXPECT_GT(camera.center().x, 400.0f);
    EXPECT_LT(camera.center().x, 600.0f);
    for (int i = 0; i < 600; ++i) {
        camera.follow({600.0f, 400.0f}, 1.0f / 60.0f, 10.0f);
    }
    EXPECT_NEAR(camera.center().x, 600.0f, 0.01f);
}

}  // namespace
