// 이 파일은 무엇을 하나요?
// 이징 공식들을 실제로 계산하는 곳이에요.
// 공식은 널리 쓰이는 표준 모양(Robert Penner의 이징 함수)을 따라요.

#include "engine/tween/easing.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace wind::engine {
namespace {

constexpr float kBackOvershoot = 1.70158f;   // OutBack이 얼마나 지나쳤다 돌아올지
constexpr float kElasticPeriod = 0.3f;       // OutElastic 출렁임 간격

float out_back(float t) {
    const float c3 = kBackOvershoot + 1.0f;
    const float u = t - 1.0f;
    return 1.0f + c3 * u * u * u + kBackOvershoot * u * u;
}

float out_elastic(float t) {
    if (t <= 0.0f || t >= 1.0f) {
        return t;  // 끝점은 정확히 0과 1
    }
    const float c4 = (2.0f * std::numbers::pi_v<float>) / kElasticPeriod;
    return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

}  // namespace

float ease(Ease kind, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    switch (kind) {
        case Ease::Linear: return t;
        case Ease::InQuad: return t * t;
        case Ease::OutQuad: return 1.0f - (1.0f - t) * (1.0f - t);
        case Ease::InOutQuad:
            return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
        case Ease::InCubic: return t * t * t;
        case Ease::OutCubic: return 1.0f - std::pow(1.0f - t, 3.0f);
        case Ease::InOutCubic:
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
        case Ease::OutBack: return out_back(t);
        case Ease::OutElastic: return out_elastic(t);
    }
    return t;
}

std::optional<Ease> parse_ease(std::string_view name) {
    if (name == "linear") return Ease::Linear;
    if (name == "in_quad") return Ease::InQuad;
    if (name == "out_quad") return Ease::OutQuad;
    if (name == "in_out_quad") return Ease::InOutQuad;
    if (name == "in_cubic") return Ease::InCubic;
    if (name == "out_cubic") return Ease::OutCubic;
    if (name == "in_out_cubic") return Ease::InOutCubic;
    if (name == "out_back") return Ease::OutBack;
    if (name == "out_elastic") return Ease::OutElastic;
    return std::nullopt;
}

}  // namespace wind::engine
