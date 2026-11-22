#pragma once

// 이 파일은 무엇을 하나요?
// 주인공 발이 출구 칸에 들어가는지 지켜보는 담당이에요. (명세 7.4, 7단계 규칙을 FieldState에서 옮겨 왔어요, 13b)
// - 출구가 아닌 칸을 한 번 밟은 뒤에만 출구가 동작해요("무장"). 그래야 출구 바로 옆에서
//   나타났을 때 곧장 되돌아가지 않아요.
// - 쓰러진 몸이 넉백으로 출구에 밀려 들어가도 떠나지 않아요 (12c). 쿠션에서 쉬는 중에도 마찬가지예요 (13b).
// - 밟은 출구는 한 번만 꺼내 갈 수 있어요. SDL을 몰라요.

#include "engine/core/math.hpp"

#include "wind/world/map.hpp"

#include <optional>

namespace wind::game {

class ExitWatcher {
public:
    // 한 박자: feet(발 위치)가 출구 칸이면 요청을 남겨요. can_leave가 false면(쓰러짐·쉬기) 보지 않아요.
    void step(const world::MapData& map, engine::Vec2 feet, bool can_leave);
    // 밟은 출구를 한 번 돌려주고 비워요. 없으면 nullopt
    [[nodiscard]] std::optional<world::MapExit> take_request();

private:
    bool armed_ = false;                      // 출구가 아닌 칸을 밟은 뒤라 출구가 동작하나
    std::optional<world::MapExit> request_;   // 밟은 출구 (게임 루프가 가져가요)
};

}  // namespace wind::game
