#pragma once

// 이 파일은 무엇을 하나요?
// 떠 있는 피해 숫자(DamageNumbers)를 글자로 그리는 도우미 함수예요. (명세 6.7, 10b)
// - 보통 피해: 하얀(따뜻한 흰색) 숫자
// - 치명타: 굵은 금색 숫자에 "!"를 붙여요 (글꼴 크기가 12픽셀 하나뿐이라 크기 대신 색·굵기·높이로 구별해요)
// 맵 위 위치를 카메라로 화면 위치로 바꿔 그려요. 조명 뒤에 그려서 밤에도 잘 보여요.

#include "engine/camera/camera2d.hpp"
#include "engine/ui/ui_painter.hpp"
#include "game/presentation/damage_numbers.hpp"

namespace wind::client {

void draw_damage_numbers(engine::UiPainter& painter, const engine::Camera2D& camera,
                         const game::DamageNumbers& numbers);

}  // namespace wind::client
