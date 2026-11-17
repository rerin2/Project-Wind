#pragma once

// 이 파일은 무엇을 하나요?
// 적(허수아비·몬스터) 머리 위 작은 HP 바를 그리는 도우미 함수 모음이에요. (9c에서 만들고 11d에서 나눔)
// - 테두리 포함 26x5 (안쪽 24x3), 머리 위 3픽셀. 빨간 HP + 크림색 "방금 깎인 만큼" 꼬리 (지연 바)
// - 가득 찬 채로 있으면 숨기고, 맞으면 보여요. 조명 뒤(Foreground)에 그려서 밤에도 보여요.
// 색과 크기 값의 이유는 docs/VISUALS.md "Training dummy" 절에 있어요.

#include "engine/camera/camera2d.hpp"
#include "engine/core/math.hpp"
#include "engine/render/render_queue.hpp"
#include "game/ui/delayed_bar.hpp"

namespace wind::game {

// 적 HP 바 설정: 가득 찬 채 2.5초면 완전히 숨고, 맵에 들어왔을 때 가득 찬 바는 처음부터 안 보여요
[[nodiscard]] DelayedBarTuning enemy_bar_tuning();

// feet: 맵 위 발 위치, head_height: 발에서 머리 꼭대기까지 (픽셀). 바가 숨겨져 있으면 아무것도 안 넣어요.
void queue_enemy_bar(const engine::Camera2D& camera, engine::Vec2 feet, float head_height, const DelayedBar& bar,
                     engine::RenderQueue& queue);

}  // namespace wind::game
