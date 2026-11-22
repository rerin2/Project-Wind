#pragma once

// 이 파일은 무엇을 하나요?
// 능력치 곡선 하나예요. (명세 8.2, 13a)
// (능력치, 값) 점 목록이고, 점 사이는 직선으로 이어요.
//   예: 최대 HP 곡선 [10 → 100], [20 → 190] 이면 생명력 15일 때 145
// - 첫 점보다 낮은 능력치는 첫 점 값, 마지막 점보다 높으면 마지막 점 값이에요 (평평하게 늘여요, 13a).
// - 점이 오름차순인지, 값이 음수가 아닌지는 불러올 때(progression_loader) 검사해요.
//   여기서는 받은 점을 믿고 계산만 해요. 점이 없으면 늘 0이에요.

#include <vector>

namespace wind::progression {

// 곡선의 점 하나
struct CurvePoint {
    int stat = 0;        // 능력치 (1~60)
    double value = 0.0;  // 그때의 값
};

class StatCurve {
public:
    StatCurve() = default;
    explicit StatCurve(std::vector<CurvePoint> points);

    // 능력치 stat일 때의 값 (점 사이는 직선으로 이어요)
    [[nodiscard]] double value_at(int stat) const;
    [[nodiscard]] const std::vector<CurvePoint>& points() const { return points_; }

private:
    std::vector<CurvePoint> points_;  // 능력치 오름차순
};

}  // namespace wind::progression
