#pragma once

// 이 파일은 무엇을 하나요?
// 카메라 "앞보기"(look-ahead)예요. (명세 6.8)
// 오른쪽으로 걸어가면 카메라가 오른쪽을 조금 더 보여 줘요.
// 가는 쪽이 더 잘 보여야 앞에 뭐가 있는지 미리 알 수 있거든요.
// 방향이 바뀌어도 휙 움직이지 않고 부드럽게 따라가요.

#include "engine/core/math.hpp"

namespace wind::engine {

class LookAhead {
public:
    // distance: 최대 몇 픽셀 앞을 볼지, sharpness: 클수록 빨리 따라가요.
    LookAhead(float distance, float sharpness);

    // direction: 가고 있는 방향(길이 0~1). 안 움직이면 (0, 0)이라 카메라가 가운데로 돌아와요.
    void update(Vec2 direction, float dt_seconds);
    // 지금 카메라를 얼마나 옮겨야 하는지
    [[nodiscard]] Vec2 offset() const { return offset_; }
    // 순간이동처럼 바로 가운데로 되돌릴 때
    void reset() { offset_ = {}; }

private:
    float distance_;
    float sharpness_;
    Vec2 offset_;
};

}  // namespace wind::engine
