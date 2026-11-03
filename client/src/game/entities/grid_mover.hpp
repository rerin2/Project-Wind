#pragma once

// 이 파일은 무엇을 하나요?
// 캐릭터를 바둑판 위의 바둑돌처럼 "한 칸씩" 옮기는 장치예요.
// - 갈 수 있는 칸인지는 맵 정보(어디가 벽이고 어디가 길인지)를 보고 정해요.
// - 화면에서는 바둑돌이 옆 칸으로 "스르륵" 미끄러지듯 보이게 해요 (이것을 보간이라고 해요).
// 지금(단계 1)은 내 컴퓨터가 혼자 판단하지만, 단계 2부터는 서버(게임의 심판 컴퓨터)가
// 최종 결정을 해요. (명세 6.2)

#include "engine/core/math.hpp"

#include "wind/world/direction.hpp"
#include "wind/world/map.hpp"

namespace wind::game {

// 한 캐릭터의 "칸 이동"을 맡는 클래스(설계도)예요.
class GridMover {
public:
    // 이동을 부탁했을 때 돌아오는 대답 세 가지
    // Started: 출발했어요 / Blocked: 벽이나 맵 끝이라 못 가요 / Busy: 아직 이동 중이라 기다려요
    enum class MoveResult { Started, Blocked, Busy };

    // 처음 서 있는 칸, 처음 바라보는 방향, 한 칸 가는 데 걸리는 시간(초)을 정해요.
    GridMover(world::TileCoord start, world::Direction facing, double step_seconds);

    // dir 방향으로 한 칸 가 보라고 부탁해요.
    // 못 가는 칸이라도, 이동 중이 아니면 그쪽으로 고개는 돌려요.
    MoveResult try_move(world::Direction dir, const world::MapData& map);

    // 시간이 dt_seconds초 흘렀다고 알려 줘요. 이동이 조금씩 진행돼요.
    // 이번에 딱 도착했다면 "남은 시간"을 돌려줘요.
    // (예: 0.2초짜리 이동인데 0.25초가 흘렀으면 0.05초가 남아요. 이걸 다음 칸 이동에 이어서 써요.)
    double update(double dt_seconds);

    // 게임 규칙상 지금 있는 칸이에요.
    // 이동 중이면 벌써 "도착할 칸"을 돌려줘요. (서버도 이렇게 생각하기로 했어요.)
    [[nodiscard]] world::TileCoord tile() const { return to_; }
    // 지금 바라보는 방향
    [[nodiscard]] world::Direction facing() const { return facing_; }
    // 지금 걸어가는 중인가요?
    [[nodiscard]] bool moving() const { return moving_; }

    // 화면에 그릴 위치예요. 칸 단위지만 소수점이 있어요.
    // 예: 1번 칸에서 2번 칸으로 반쯤 갔으면 1.5
    [[nodiscard]] engine::Vec2 visual_tile() const;

    // 걸어가지 않고 "순간이동"으로 바로 그 칸에 놓아요. (다른 맵으로 갈 때 등)
    void teleport(world::TileCoord tile);

private:
    world::TileCoord from_;   // 출발한 칸
    world::TileCoord to_;     // 도착할 칸
    world::Direction facing_; // 바라보는 방향
    double step_seconds_;     // 한 칸 가는 데 걸리는 시간
    double elapsed_ = 0.0;    // 이번 칸 이동을 시작하고 지난 시간
    bool moving_ = false;     // 걷는 중이면 true
};

}  // namespace wind::game
