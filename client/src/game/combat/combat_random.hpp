#pragma once

// 이 파일은 무엇을 하나요?
// 싸움 전용 주사위예요. (명세 9.3 "시드가 있는 게임 전용 난수기")
// - 같은 씨앗(seed)이면 언제 어디서나 같은 순서로 같은 값이 나와요. 그래서 테스트로 재현할 수 있어요.
// - std::uniform_real_distribution은 컴파일러마다 값이 달라질 수 있어서 쓰지 않고,
//   간단하고 널리 알려진 splitmix64 계산을 직접 해요.
// - 연출용 파티클 주사위(ParticleRandom)와 따로 써요. 연출이 주사위를 굴려도 싸움 결과가 바뀌지 않게요.

#include <cstdint>

namespace wind::game {

class CombatRandom {
public:
    explicit CombatRandom(std::uint64_t seed);

    // 0 이상 1 미만의 수
    [[nodiscard]] float next_unit();
    // chance(0~1) 확률로 true. 0 이하면 늘 false, 1 이상이면 늘 true (주사위는 그래도 한 번 굴려요)
    [[nodiscard]] bool roll(float chance);

private:
    std::uint64_t state_;
};

}  // namespace wind::game
