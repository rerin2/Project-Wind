// 이 파일은 무엇을 하나요?
// direction.hpp에서 약속한 일(한 칸 옮기기, 방향 <-> 글자 바꾸기)을 실제로 해요.

#include "wind/world/direction.hpp"

namespace wind::world {

// 방향에 따라 x나 y를 1씩 더하거나 빼요.
// y는 아래로 갈수록 커지니까, 위(Up)로 가면 y가 1 줄어들어요.
TileCoord step(TileCoord from, Direction dir) {
    switch (dir) {
        case Direction::Down: return {from.x, from.y + 1};
        case Direction::Left: return {from.x - 1, from.y};
        case Direction::Right: return {from.x + 1, from.y};
        case Direction::Up: return {from.x, from.y - 1};
    }
    return from;  // 여기까지 올 일은 없지만, 컴파일러(코드 번역기)가 걱정하지 않게 적어 둬요
}

// 방향 하나를 짧은 영어 글자로 바꿔요. 맵·그림 설명서(JSON)에 이 글자를 써요.
std::string_view to_string(Direction dir) {
    switch (dir) {
        case Direction::Down: return "down";
        case Direction::Left: return "left";
        case Direction::Right: return "right";
        case Direction::Up: return "up";
    }
    return "down";
}

// 글자를 보고 어느 방향인지 찾아요. 네 방향의 이름과 하나씩 비교해 봐요.
// 대문자("Down")는 다른 글자로 봐서 찾지 못해요.
std::optional<Direction> parse_direction(std::string_view text) {
    for (Direction dir : kAllDirections) {
        if (to_string(dir) == text) {
            return dir;
        }
    }
    return std::nullopt;  // 아무것도 맞지 않으면 "없음"
}

}  // namespace wind::world
