#pragma once

// 이 파일은 무엇을 하나요?
// 잔상(afterimage) = 구를 때 캐릭터 뒤에 남는 흐릿한 그림자 모습이에요.
// 아주 빨리 움직인다는 느낌을 줘요. (명세 6.7)
// 구르는 동안 짧은 간격으로 "사진"을 찍어 두고, 사진은 금방 흐려지며 사라져요.

#include "engine/camera/camera2d.hpp"
#include "engine/core/math.hpp"
#include "engine/render/render_queue.hpp"

#include <vector>

namespace wind::game {

class AfterimageTrail {
public:
    // 사진 한 장: 어디에, 어떤 그림 조각을 그렸었는지
    struct Snapshot {
        engine::Vec2 feet;
        engine::RectI source;  // 그림 판에서 오려 낸 칸
        float age = 0.0f;
    };

    // recording: 지금 사진을 찍어야 하나 (구르는 중이면 true)
    void update(float dt_seconds, bool recording, engine::Vec2 feet, engine::RectI source);
    // 사진들을 흐릿하게 그리기 목록에 넣어요.
    // origin: 그림 칸 안의 발 위치, frame_size: 그림 칸 크기
    void queue(const engine::Camera2D& camera, engine::TextureHandle texture, engine::Vec2i origin,
               engine::Vec2i frame_size, engine::RenderQueue& queue) const;
    void clear() { snapshots_.clear(); }

    [[nodiscard]] const std::vector<Snapshot>& snapshots() const { return snapshots_; }

private:
    std::vector<Snapshot> snapshots_;
    float since_last_ = 0.0f;  // 마지막 사진 뒤 흐른 시간
};

}  // namespace wind::game
