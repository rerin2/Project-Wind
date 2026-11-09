#pragma once

// 이 파일은 무엇을 하나요?
// 방향키를 보고 "어느 쪽으로 가고 싶은지"를 화살표(Vec2) 하나로 만들어요.
// 위+오른쪽을 같이 누르면 대각선(↗)이에요. 8방향으로 갈 수 있어요.
// 화살표 길이는 항상 1(또는 0)이라서, 대각선이라고 더 빨리 가지 않아요.
//
// 반대 방향 키를 같이 누르면(예: 왼쪽+오른쪽) "마지막에 누른 키"가 이겨요.
// 그래야 왼쪽을 누른 채 오른쪽을 눌렀을 때 멈추지 않고 바로 오른쪽으로 가요.

#include "engine/core/math.hpp"
#include "engine/input/input_state.hpp"
#include "game/input/input_bindings.hpp"

namespace wind::game {

class MovementIntent {
public:
    // 화면 한 장마다 불러서, 새로 누른 방향 키를 기억해요.
    void update(const InputBindings& bindings, const engine::InputState& input);

    // 가고 싶은 방향 (길이 1). 방향키를 안 누르고 있으면 (0, 0)
    [[nodiscard]] engine::Vec2 direction(const InputBindings& bindings,
                                         const engine::InputState& input) const;

private:
    // 가로축, 세로축에서 각각 마지막으로 누른 쪽: -1(왼쪽/위), +1(오른쪽/아래), 0(없음)
    int last_x_ = 0;
    int last_y_ = 0;
};

}  // namespace wind::game
