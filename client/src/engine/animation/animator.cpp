// 이 파일은 무엇을 하나요?
// 애니메이션 플레이어(Animator)가 시간에 맞춰 그림을 넘기는 방법을 실제로 적은 곳이에요.

#include "engine/animation/animator.hpp"

#include <cmath>
#include <numeric>

namespace wind::engine {

void Animator::play(const AnimationClip* clip, bool restart) {
    if (clip == clip_ && !restart) {
        return;  // 같은 클립을 매번 틀어도 처음으로 돌아가지 않고 하던 곳부터 계속해요
    }
    clip_ = clip;
    frame_ = 0;
    elapsed_ = 0.0;
    finished_ = false;
}

void Animator::update(double dt_seconds) {
    if (clip_ == nullptr || clip_->frames.empty() || finished_) {
        return;  // 틀고 있는 게 없거나 이미 끝났으면 할 일 없음
    }
    elapsed_ += dt_seconds;

    // 반복 클립인데 시간이 클립 전체 길이보다 많이 흘렀다면,
    // 몇 바퀴 돌았는지는 중요하지 않으니 "나머지 시간"만 남겨요. (계산을 짧게 하려고)
    if (clip_->loop) {
        const double total = std::accumulate(clip_->seconds.begin(), clip_->seconds.end(), 0.0);
        if (total > 0.0 && elapsed_ > total) {
            elapsed_ = std::fmod(elapsed_, total);
        }
    }

    // 지금 그림을 보여 줄 시간이 다 지났으면 다음 그림으로 넘겨요.
    while (elapsed_ >= clip_->seconds[frame_]) {
        elapsed_ -= clip_->seconds[frame_];
        if (frame_ + 1 < clip_->frames.size()) {
            ++frame_;  // 다음 그림
        } else if (clip_->loop) {
            frame_ = 0;  // 처음으로 돌아가기
        } else {
            finished_ = true;  // 반복 안 하는 클립은 마지막 그림에서 멈춰요
            elapsed_ = 0.0;
            break;
        }
    }
}

// 지금 그림이 그림 판의 몇 번째 칸인지 알려 줘요.
int Animator::column() const {
    if (clip_ == nullptr || clip_->frames.empty()) {
        return 0;
    }
    return clip_->frames[frame_];
}

}  // namespace wind::engine
