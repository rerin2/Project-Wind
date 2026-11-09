// 이 파일은 무엇을 하나요?
// 눌린 방향키로 "가고 싶은 방향 화살표"를 계산하는 곳이에요.

#include "game/input/movement_intent.hpp"

namespace wind::game {
namespace {

// 한 축(가로 또는 세로)의 값을 정해요.
// negative = 왼쪽(또는 위) 키, positive = 오른쪽(또는 아래) 키
// 하나만 누르면 그쪽, 둘 다 누르면 마지막에 누른 쪽, 둘 다 안 누르면 0
float axis(bool negative, bool positive, int last) {
    if (negative && positive) {
        return static_cast<float>(last);
    }
    if (negative) {
        return -1.0f;
    }
    if (positive) {
        return 1.0f;
    }
    return 0.0f;
}

}  // namespace

void MovementIntent::update(const InputBindings& bindings, const engine::InputState& input) {
    // 이번 화면에서 "새로" 누른 키가 있으면 그쪽을 마지막 방향으로 기억해요.
    if (bindings.pressed(Action::MoveLeft, input)) {
        last_x_ = -1;
    }
    if (bindings.pressed(Action::MoveRight, input)) {
        last_x_ = 1;
    }
    if (bindings.pressed(Action::MoveUp, input)) {
        last_y_ = -1;
    }
    if (bindings.pressed(Action::MoveDown, input)) {
        last_y_ = 1;
    }
}

engine::Vec2 MovementIntent::direction(const InputBindings& bindings,
                                       const engine::InputState& input) const {
    const engine::Vec2 raw{
        axis(bindings.held(Action::MoveLeft, input), bindings.held(Action::MoveRight, input), last_x_),
        axis(bindings.held(Action::MoveUp, input), bindings.held(Action::MoveDown, input), last_y_),
    };
    // 대각선 (1, 1)은 길이가 1.41이라, 길이 1로 줄여서 대각선이 더 빠르지 않게 해요.
    return engine::normalized(raw);
}

}  // namespace wind::game
