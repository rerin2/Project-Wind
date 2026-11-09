#pragma once

// 이 파일은 무엇을 하나요?
// 파티클용 주사위예요. 점마다 수명·크기·방향을 조금씩 다르게 해서 자연스럽게 보이게 해요.
// 시작 번호(씨앗)를 정하면 매번 같은 순서로 나와요. 그래서 테스트에서 결과를 맞춰 볼 수 있어요.
// 화면 연출 전용이에요. 게임 규칙용 주사위(단계 11)와는 따로 써요.

#include <cstdint>
#include <random>

namespace wind::engine {

class ParticleRandom {
public:
    explicit ParticleRandom(std::uint32_t seed);

    // a ~ b 사이의 소수. a와 b가 같으면 그 값 그대로.
    [[nodiscard]] float range(float a, float b);

private:
    std::minstd_rand engine_;
};

}  // namespace wind::engine
