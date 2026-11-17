#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터가 지금 무엇을 하는지(상태) 이름표예요. (명세 9.6)
// 11a에서는 대기와 배회만 있어요. 발견·추적·공격 예고·공격·후딜·복귀는 11b에서, 사망은 11d에서 더해요.
// 몬스터 그림(MonsterView)과 F3 정보판도 이 이름표를 봐서 따로 파일로 뒀어요.

#include <cstdint>
#include <string_view>

namespace wind::game {

enum class MonsterState : std::uint8_t {
    Idle,    // 제자리 근처에서 잠깐 쉬어요
    Wander,  // 제자리 둘레의 한 점으로 천천히 걸어가요
};

// F3 정보판·로그용 영어 이름 ("idle", "wander")
[[nodiscard]] constexpr std::string_view to_string(MonsterState state) {
    switch (state) {
        case MonsterState::Idle: return "idle";
        case MonsterState::Wander: return "wander";
    }
    return "?";
}

}  // namespace wind::game
