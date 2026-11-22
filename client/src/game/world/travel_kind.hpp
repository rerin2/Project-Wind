#pragma once

// 이 파일은 무엇을 하나요?
// 맵을 옮기는 이유(종류)예요. (12c 부활, 13b 쉬기)
// 종류에 따라 새 필드에서 하는 일이 달라요:
//   Exit    : 출구를 밟았어요. 주인공 HP·MP·스태미나를 들고 가요.
//   Respawn : 쓰러져서 부활해요. 가득 찬 채 마지막 쿠션(없으면 맵 기본 자리)에서 깨어나요.
//   Rest    : 캣닢 쿠션에서 쉬어요. 같은 맵을 새로 읽어 몬스터가 다시 나오고, 쿠션에 앉은 채 시작해요.

#include <cstdint>
#include <string_view>

namespace wind::game {

enum class TravelKind : std::uint8_t { Exit, Respawn, Rest };

// 로그용 이름 ("Exit" 등)
[[nodiscard]] constexpr std::string_view to_string(TravelKind kind) {
    switch (kind) {
        case TravelKind::Exit: return "Exit";
        case TravelKind::Respawn: return "Respawn";
        case TravelKind::Rest: return "Rest";
    }
    return "Travel";  // 여기 올 일은 없어요
}

}  // namespace wind::game
