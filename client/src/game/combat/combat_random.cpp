// 이 파일은 무엇을 하나요?
// splitmix64 계산으로 싸움 주사위를 굴리는 곳이에요.

#include "game/combat/combat_random.hpp"

namespace wind::game {
namespace {

// splitmix64에 쓰는 정해진 수들 (Sebastiano Vigna가 공개한 값)
constexpr std::uint64_t kGamma = 0x9E3779B97F4A7C15ULL;
constexpr std::uint64_t kMix1 = 0xBF58476D1CE4E5B9ULL;
constexpr std::uint64_t kMix2 = 0x94D049BB133111EBULL;
// float에 정확히 담기는 24비트만 써서 [0, 1)을 만들어요 (1.0이 나오지 않게)
constexpr int kFloatBits = 24;
constexpr float kUnitScale = 1.0f / static_cast<float>(1U << kFloatBits);

}  // namespace

CombatRandom::CombatRandom(std::uint64_t seed) : state_(seed) {}

float CombatRandom::next_unit() {
    // splitmix64: 상태를 일정하게 더하고, 비트를 섞어서 고르게 퍼진 수를 만들어요
    state_ += kGamma;
    std::uint64_t z = state_;
    z = (z ^ (z >> 30)) * kMix1;  // 줄마다 비트 섞기
    z = (z ^ (z >> 27)) * kMix2;
    z ^= z >> 31;
    return static_cast<float>(z >> (64 - kFloatBits)) * kUnitScale;  // 위쪽 24비트 -> 0~1
}

bool CombatRandom::roll(float chance) {
    const float value = next_unit();  // 확률과 상관없이 늘 한 번 굴려서 순서가 흔들리지 않게
    return value < chance;
}

}  // namespace wind::game
