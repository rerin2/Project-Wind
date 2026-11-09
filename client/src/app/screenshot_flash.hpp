#pragma once

// 이 파일은 무엇을 하나요?
// 스크린샷을 찍은 뒤 보여 주는 "찰칵" 효과예요.
// 화면이 하얗게 번쩍했다가 사르르 사라지고, 구석에 "screenshot saved" 글자가 잠깐 떠요.
// 사진에는 이 효과가 찍히지 않도록, 사진을 저장한 "다음 화면"부터 보여 줘요.

#include "engine/render/renderer.hpp"
#include "engine/tween/tween.hpp"

namespace wind::client {

class ScreenshotFlash {
public:
    ScreenshotFlash();

    // 효과를 처음부터 시작해요.
    void trigger();
    void update(float dt_seconds);
    // 효과가 진행 중이면 화면 맨 위에 그려요.
    void draw(engine::Renderer& renderer) const;

private:
    engine::Tween flash_;  // 하얀 번쩍임 진하기 (1 -> 0)
    engine::Tween label_;  // 글자 진하기 (1 -> 0, 더 천천히)
    bool active_ = false;
};

}  // namespace wind::client
