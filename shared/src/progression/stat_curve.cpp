// 이 파일은 무엇을 하나요?
// 능력치 곡선에서 값을 읽어요 (점 사이 직선 잇기).

#include "wind/progression/stat_curve.hpp"

#include <utility>

namespace wind::progression {

StatCurve::StatCurve(std::vector<CurvePoint> points) : points_(std::move(points)) {}

double StatCurve::value_at(int stat) const {
    if (points_.empty()) return 0.0;                              // 점이 없으면 0
    if (stat <= points_.front().stat) return points_.front().value;  // 첫 점보다 낮으면 첫 값
    if (stat >= points_.back().stat) return points_.back().value;    // 마지막 점보다 높으면 마지막 값
    // stat을 사이에 둔 두 점 a, b를 찾아요
    for (std::size_t i = 1; i < points_.size(); ++i) {
        const CurvePoint& a = points_[i - 1];
        const CurvePoint& b = points_[i];
        if (stat > b.stat) continue;                              // 아직 더 오른쪽
        // a에서 b까지 얼마나 왔나 (0~1). 점이 오름차순이라 b.stat > a.stat 이에요.
        const double t = static_cast<double>(stat - a.stat) / static_cast<double>(b.stat - a.stat);
        return a.value + (b.value - a.value) * t;                 // 직선 위의 값
    }
    return points_.back().value;  // 점이 오름차순이면 여기 오지 않아요
}

}  // namespace wind::progression
