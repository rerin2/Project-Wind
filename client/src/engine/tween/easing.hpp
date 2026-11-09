#pragma once

// 이 파일은 무엇을 하나요?
// 이징(easing) = "움직임의 느낌"을 정하는 공식이에요. (명세 5.6)
// 0에서 1로 갈 때, 일정하게 갈 수도 있고(Linear), 처음엔 천천히 갔다가 빨라질 수도 있고(In),
// 빨리 출발해서 끝에 사뿐히 멈출 수도 있어요(Out).
// t(0~1, 얼마나 진행됐는지)를 넣으면 "얼마나 갔는지"(보통 0~1)를 돌려줘요.
// Back과 Elastic은 끝에서 살짝 넘쳤다가 돌아와서 1보다 커질 때가 있어요 (통통 튀는 느낌).

#include <cstdint>
#include <optional>
#include <string_view>

namespace wind::engine {

enum class Ease : std::uint8_t {
    Linear,       // 일정한 빠르기
    InQuad,       // 천천히 출발 (약하게)
    OutQuad,      // 사뿐히 멈춤 (약하게)
    InOutQuad,    // 천천히 출발, 사뿐히 멈춤
    InCubic,      // 천천히 출발 (강하게)
    OutCubic,     // 사뿐히 멈춤 (강하게)
    InOutCubic,
    OutBack,      // 목표를 살짝 지나쳤다 돌아옴 (UI 창이 "톡" 나타날 때)
    OutElastic,   // 고무줄처럼 출렁이다 멈춤
};

// t를 0~1로 자른 다음 공식을 적용해요. 시작(0)은 항상 0, 끝(1)은 항상 1이에요.
[[nodiscard]] float ease(Ease kind, float t);

// 자료 파일에서 쓰는 이름("out_quad" 등)을 Ease로 바꿔요. 모르는 이름이면 nullopt.
[[nodiscard]] std::optional<Ease> parse_ease(std::string_view name);

}  // namespace wind::engine
