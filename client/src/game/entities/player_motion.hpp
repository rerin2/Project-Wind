#pragma once

// 이 파일은 무엇을 하나요?
// 주인공의 몸 움직임 규칙을 한데 묶어요. (명세 7.2, 7.2.1)
// - Mover(다리): 걷기, 달리기, 구르기, 벽 미끄러짐
// - Stamina(기력): 달리기·구르기에 쓰고, 쉬면 다시 참
// - InputBuffer(버튼 메모장): 구르기 버튼을 0.15초 기억
// 한 박자 순서:
//   1) 구르기 메모가 있고, 구를 수 있고, 스태미나가 25 이상이면 -> 구르기 (스태미나 25 씀)
//   2) Shift를 누르고 움직이며 지침이 아니면 -> 달리기 (달리는 동안 1초에 25 씀)
//   3) 쓰지 않았으면 0.6초 뒤부터 다시 참
// SDL을 몰라서 창 없이 테스트할 수 있어요.

#include "engine/core/math.hpp"
#include "game/entities/mover.hpp"
#include "game/entities/stamina.hpp"
#include "game/input/input_buffer.hpp"

#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

namespace wind::game {

class PlayerMotion {
public:
    // buffer_seconds: 구르기 버튼을 몇 초 동안 기억할지
    PlayerMotion(engine::Vec2 feet, world::Direction facing, MovementTuning movement,
                 StaminaTuning stamina, double buffer_seconds);

    // 구르기 버튼을 눌렀어요 (화면 한 장마다, 박자 계산 전에). 메모장에 적어 둬요.
    void press_dodge();
    // 한 박자. move: 가고 싶은 방향, run_held: Shift를 누르고 있나
    // 구르기를 시작했으면 true
    bool step(double dt_seconds, engine::Vec2 move, bool run_held, const world::MapData& map);

    [[nodiscard]] const Mover& mover() const { return mover_; }
    [[nodiscard]] const Stamina& stamina() const { return stamina_; }

private:
    Mover mover_;
    Stamina stamina_;
    InputBuffer buffer_;
};

}  // namespace wind::game
