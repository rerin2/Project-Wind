#pragma once

// 이 파일은 무엇을 하나요?
// 클립 = 동작 하나(예: "걷기")에 쓰는 그림 순서와 각 그림을 보여 줄 시간이에요. (명세 5.6)
// 공책 모서리에 그림을 조금씩 다르게 그려서 휘리릭 넘기면 움직이는 것처럼 보이죠?
// 그 그림 한 장 한 장을 "프레임"이라고 해요.

#include <optional>
#include <vector>

namespace wind::engine {

struct AnimationClip {
    std::vector<int> frames;      // 그림 판에서 몇 번째 칸(열)을 쓸지, 순서대로
    std::vector<double> seconds;  // frames와 같은 개수. 각 그림을 몇 초 보여 줄지
    bool loop = true;             // true면 끝나고 처음부터 다시 (걷기), false면 끝에서 멈춤
    std::optional<int> row;       // 정해진 행 (13b: 쿠션 동작처럼 방향과 상관없는 그림). 없으면 바라보는 방향의 행
};

}  // namespace wind::engine
