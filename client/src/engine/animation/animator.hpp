#pragma once

// 이 파일은 무엇을 하나요?
// 애니메이터 = 클립을 재생하는 "플레이어"예요. 시간이 흐르면 다음 그림으로 넘겨요.
// "지금은 몇 번째 그림"인지만 계산해요. 공격이 맞았는지 같은 판정은 게임 규칙 쪽이 해요.

#include "engine/animation/animation_clip.hpp"

#include <cstddef>

namespace wind::engine {

class Animator {
public:
    // 클립을 틀어요. 다른 클립이면 처음부터 시작해요.
    // 같은 클립이면 하던 곳부터 계속하고, restart가 true일 때만 처음으로 돌아가요.
    void play(const AnimationClip* clip, bool restart = false);
    // 시간이 dt_seconds초 흘렀다고 알려 줘요.
    void update(double dt_seconds);

    // 지금 보여 줄 그림의 칸 번호 (클립이 없으면 0)
    [[nodiscard]] int column() const;
    // 반복하지 않는 클립이 마지막 그림까지 다 끝났나요?
    [[nodiscard]] bool finished() const { return finished_; }
    [[nodiscard]] const AnimationClip* clip() const { return clip_; }

private:
    const AnimationClip* clip_ = nullptr;  // 지금 틀고 있는 클립
    std::size_t frame_ = 0;                // 클립 안에서 몇 번째 그림인지
    double elapsed_ = 0.0;                 // 지금 그림을 보여 준 지 몇 초 됐는지
    bool finished_ = false;
};

}  // namespace wind::engine
