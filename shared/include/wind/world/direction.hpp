#pragma once

// 이 파일은 무엇을 하나요?
// 게임 맵은 바둑판처럼 생겼어요. 이 파일은 "바둑판의 몇 번째 칸인지"(타일 좌표)와
// "어느 쪽으로 가는지"(방향)를 정해 둡니다.
// 게임 규칙은 언제나 정수 칸 번호로만 계산해요 (명세 5.3).
// 클라이언트(게임 화면)와 서버(심판 컴퓨터)가 똑같은 규칙을 쓰도록 shared(공용 폴더)에 둡니다.

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace wind::world {

// 캐릭터가 갈 수 있는 네 방향: 아래, 왼쪽, 오른쪽, 위 (명세 5.6).
// enum(이넘: 정해진 보기 중 하나만 고르는 목록)이라 다른 값은 들어갈 수 없어요.
// 이 순서는 그림 파일(스프라이트 시트)의 줄 순서와는 상관없어요.
enum class Direction : std::uint8_t { Down, Left, Right, Up };

// 네 방향을 한 번씩 차례로 살펴볼 때 쓰는 목록
inline constexpr std::array kAllDirections{Direction::Down, Direction::Left, Direction::Right,
                                           Direction::Up};

// 바둑판의 한 칸 위치.
// (0,0)은 왼쪽 맨 위 칸이에요. x는 오른쪽으로 갈수록, y는 아래로 갈수록 커져요.
struct TileCoord {
    std::int32_t x = 0;
    std::int32_t y = 0;

    // 두 위치의 x와 y가 모두 같으면 "같은 칸"이에요.
    friend bool operator==(const TileCoord&, const TileCoord&) = default;
};

// from 칸에서 dir 방향으로 딱 한 칸 옮긴 위치를 알려 줘요.
[[nodiscard]] TileCoord step(TileCoord from, Direction dir);

// 방향을 글자로 바꾸거나("down", "left", "right", "up"), 글자를 방향으로 바꿔요.
// 모르는 글자가 들어오면 parse_direction은 "없음"(nullopt)을 돌려줘요.
[[nodiscard]] std::string_view to_string(Direction dir);
[[nodiscard]] std::optional<Direction> parse_direction(std::string_view text);

}  // namespace wind::world
