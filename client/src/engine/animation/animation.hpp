#pragma once

// 이 파일은 무엇을 하나요?
// 캐릭터가 걷는 모습 같은 "움직이는 그림(애니메이션)"을 다뤄요. (명세 5.6)
// 공책 모서리에 그림을 조금씩 다르게 그려서 휘리릭 넘기면 움직이는 것처럼 보이죠?
// 그 그림 한 장 한 장을 "프레임"이라고 해요.
// 어떤 그림을 몇 초씩 보여 줄지는 설명서(JSON 파일)에 적혀 있고,
// 이 파일은 시간에 맞춰 "지금은 몇 번째 그림"인지만 계산해요.
// 애니메이션은 보여 주기용일 뿐, 공격이 맞았는지 같은 판정은 서버가 해요.

#include "engine/core/math.hpp"

#include "wind/world/direction.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace wind::engine {

// 클립 = 동작 하나(예: "걷기")에 쓰는 그림 순서와 각 그림을 보여 줄 시간
struct AnimationClip {
    std::vector<int> frames;      // 그림 판에서 몇 번째 칸(열)을 쓸지, 순서대로
    std::vector<double> seconds;  // frames와 같은 개수. 각 그림을 몇 초 보여 줄지
    bool loop = true;             // true면 끝나고 처음부터 다시 (걷기), false면 끝에서 멈춤
};

// 스프라이트 시트 = 캐릭터 그림을 표처럼 모아 둔 한 장의 그림
// 가로(열)는 동작 순서, 세로(행)는 방향이에요. 예: 1행 = 아래를 보는 모습
struct SpriteSheet {
    std::string id;
    std::string image;  // 그림 파일 위치 (콘텐츠 폴더 기준)
    std::int32_t frame_width = 0;   // 그림 한 칸의 가로 크기
    std::int32_t frame_height = 0;  // 그림 한 칸의 세로 크기
    // 그림 한 칸 안에서 "발"이 있는 점. 캐릭터의 맵 위치에 이 점을 맞춰서 그려요.
    Vec2i origin;
    std::array<std::int32_t, 4> direction_rows{};  // 방향(아래·왼쪽·오른쪽·위)마다 몇 번째 행인지
    std::map<std::string, AnimationClip, std::less<>> clips;  // 이름 -> 클립 ("idle", "walk" ...)

    // 이름으로 클립을 찾아요. 없으면 nullptr(빈손)
    [[nodiscard]] const AnimationClip* clip(std::string_view name) const;
    // 방향과 칸 번호로, 그림 판에서 오려 낼 네모를 알려 줘요.
    [[nodiscard]] RectI frame_rect(world::Direction dir, int column) const;
};

// 애니메이터 = 클립을 재생하는 "플레이어". 시간이 흐르면 다음 그림으로 넘겨요.
class Animator {
public:
    // 클립을 틀어요. 다른 클립이면 처음부터 시작해요.
    // 같은 클립이면 하던 곳부터 계속하고, restart가 true일 때만 처음으로 돌아가요.
    void play(const AnimationClip* clip, bool restart = false);
    // 시간이 dt_seconds초 흘렀다고 알려 줘요.
    void update(double dt_seconds);

    // 지금 보여 줄 그림의 칸 번호 (클립이 없으면 0)
    [[nodiscard]] int column() const;
    // 반복하지 않는 클립이 마지막 그림까지 다 끝났나요?
    [[nodiscard]] bool finished() const { return finished_; }
    [[nodiscard]] const AnimationClip* clip() const { return clip_; }

private:
    const AnimationClip* clip_ = nullptr;  // 지금 틀고 있는 클립
    std::size_t frame_ = 0;                // 클립 안에서 몇 번째 그림인지
    double elapsed_ = 0.0;                 // 지금 그림을 보여 준 지 몇 초 됐는지
    bool finished_ = false;
};

}  // namespace wind::engine
