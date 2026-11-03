// 이 파일은 무엇을 하나요?
// grid_mover.hpp에서 약속한 "한 칸씩 옮기기"가 실제로 어떻게 동작하는지 적은 곳이에요.

#include "game/entities/grid_mover.hpp"

#include <algorithm>

namespace wind::game {

// 처음에는 출발 칸과 도착 칸이 같아요. (가만히 서 있는 상태)
GridMover::GridMover(world::TileCoord start, world::Direction facing, double step_seconds)
    : from_(start), to_(start), facing_(facing), step_seconds_(step_seconds) {}

GridMover::MoveResult GridMover::try_move(world::Direction dir, const world::MapData& map) {
    if (moving_) {
        return MoveResult::Busy;  // 한 칸 이동이 끝나기 전에는 새 이동을 받지 않는다
    }
    facing_ = dir;  // 먼저 그쪽을 바라봐요
    const world::TileCoord target = world::step(to_, dir);  // 가고 싶은 옆 칸
    if (!map.is_walkable(target)) {
        return MoveResult::Blocked;  // 맵 밖이거나 막힌 칸 (벽, 물, 나무 밑동...)
    }
    // 출발! 지금 칸이 출발 칸이 되고, 옆 칸이 도착 칸이 돼요.
    from_ = to_;
    to_ = target;
    elapsed_ = 0.0;
    moving_ = true;
    return MoveResult::Started;
}

double GridMover::update(double dt_seconds) {
    if (!moving_) {
        return 0.0;  // 서 있으면 할 일이 없어요
    }
    elapsed_ += dt_seconds;  // 흐른 시간을 더해요
    if (elapsed_ < step_seconds_) {
        return 0.0;  // 아직 도착 전
    }
    // 도착: 넘친 시간은 돌려줘서 다음 칸 이동에 이어 쓴다 (끊김 방지).
    // 남은 시간을 버리면 칸마다 아주 살짝 멈칫해서 걸음이 덜컹거려 보여요.
    const double leftover = elapsed_ - step_seconds_;
    from_ = to_;
    elapsed_ = 0.0;
    moving_ = false;
    return leftover;
}

engine::Vec2 GridMover::visual_tile() const {
    // t는 "얼마나 왔나"를 0~1로 나타낸 값이에요. (0 = 출발 칸, 1 = 도착 칸, 0.5 = 딱 중간)
    const auto t = static_cast<float>(moving_ ? std::clamp(elapsed_ / step_seconds_, 0.0, 1.0) : 1.0);
    const engine::Vec2 from{static_cast<float>(from_.x), static_cast<float>(from_.y)};
    const engine::Vec2 to{static_cast<float>(to_.x), static_cast<float>(to_.y)};
    // 출발 칸에서 도착 칸 쪽으로 t만큼 간 곳 = 스르륵 미끄러지는 위치
    return from + (to - from) * t;
}

void GridMover::teleport(world::TileCoord tile) {
    // 출발 칸과 도착 칸을 둘 다 새 칸으로 바꾸고, 걷기를 멈춰요.
    from_ = tile;
    to_ = tile;
    elapsed_ = 0.0;
    moving_ = false;
}

}  // namespace wind::game
