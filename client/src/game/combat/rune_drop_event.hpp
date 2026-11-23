#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터를 쓰러뜨려 룬이 나왔다는 기록이에요. (명세 8.3, 13c)
// FieldCombat이 쓰러진 박자에 적고(한 마리 한 번), 연출(RuneOrbs)이 보고 그 자리에서
// 금빛 알갱이가 튀어나와 주인공에게 빨려 들어가게 해요. 룬 숫자는 이미 지갑에 들어가 있어요.
// 14b: 룬이 0인 몬스터도 쓰러지면 한 번 적혀요. FieldLoot가 몬스터 번호로 드롭 표를 찾아 굴려요.

#include "engine/core/math.hpp"

#include <cstddef>
#include <cstdint>

namespace wind::game {

struct RuneDropEvent {
    engine::Vec2 at;           // 쓰러진 몬스터의 발 위치 (맵 위 픽셀)
    std::int64_t amount = 0;   // 나온 룬 (rune_reward)
    std::size_t monster = 0;   // 쓰러진 몬스터 번호 (FieldCombat::monsters() 안, 14b)
};

}  // namespace wind::game
