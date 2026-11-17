#pragma once

// 이 파일은 무엇을 하나요?
// 쓰러진 몬스터가 이번 박자에 다 사라졌다는 기록이에요. (명세 9.5, 11d)
// FieldCombat이 적고, 연출(FieldCombatFx)이 보고 그 자리에서 사라짐 파티클(death_fx)을 터뜨려요.
// 나중에 룬(13단계)·드롭(14단계)도 이 순간을 써요.

#include "engine/core/math.hpp"

#include <cstddef>

namespace wind::game {

struct MonsterVanishEvent {
    std::size_t index = 0;  // FieldCombat::monsters() 안의 몇 번째 몬스터인지
    engine::Vec2 at;        // 사라진 발 위치 (맵 위 픽셀)
};

}  // namespace wind::game
