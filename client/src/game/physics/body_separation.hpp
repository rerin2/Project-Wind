#pragma once

// 이 파일은 무엇을 하나요?
// 두 몸(주인공·몬스터)이 겹쳤을 때 서로 떼어 놓을 거리를 계산하는 도우미 함수예요. (명세 7.3, 11c)
// - 몸은 발 위치를 가운데로 한 동그라미로 봐요 (반지름 = 발 상자 가로의 절반).
// - 겹친 만큼 떼어 놓되, 한 박자에 max_step 픽셀까지만 (밀어내기 힘 제한). 그래서 툭 튀지 않고 부드럽게 밀려요.
// - 무거운 쪽은 덜, 가벼운 쪽은 더 밀려요 (weight가 클수록 많이 밀려요).
// 실제로 옮기는 일(벽 미끄러짐)은 부르는 쪽이 해요. 순수한 계산이라 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"

namespace wind::game {

// 밀어내기용 몸 하나
struct BodyCircle {
    engine::Vec2 center;  // 발 위치 (맵 위 픽셀)
    float radius = 0.0f;  // 몸 반지름 (픽셀)
    float weight = 1.0f;  // 밀리는 몫 (클수록 많이 밀려요, 0이면 안 밀려요)
};

// 두 몸이 옮겨 갈 거리
struct SeparationPush {
    engine::Vec2 first;   // 첫째 몸이 옮길 거리
    engine::Vec2 second;  // 둘째 몸이 옮길 거리
};

// a와 b가 겹쳤으면 떼어 놓을 거리를 돌려줘요. 안 겹쳤으면 둘 다 0이에요.
// 발 위치가 똑같으면 정해진 방향(오른쪽)으로 떼어 놓아요 (주사위 없이 늘 같은 결과).
[[nodiscard]] SeparationPush separate_bodies(const BodyCircle& a, const BodyCircle& b, float max_step);

}  // namespace wind::game
