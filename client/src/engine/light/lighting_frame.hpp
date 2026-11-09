#pragma once

// 이 파일은 무엇을 하나요?
// 이번 화면의 조명 준비물이에요. 렌더러가 이걸 받아서 화면에 어둠과 빛을 입혀요.
// - ambient: 빛이 없는 곳의 밝기와 색 (예: 동굴 = 아주 어두운 남색)
// - lights : 화면에 보이는 빛들을 "그리기 명령"으로 바꿔 둔 것 (더하기 섞기로 그려요)

#include "engine/render/render_types.hpp"

#include <cstdint>
#include <vector>

namespace wind::engine {

struct LightingFrame {
    bool enabled = false;  // false면 조명을 쓰지 않아요 (밝은 낮 맵)
    Color ambient{255, 255, 255, 255};
    std::vector<SpriteDraw> lights;
};

// 빛 통계 (디버그 화면용)
struct LightStats {
    std::uint32_t total = 0;      // 맵에 있는 빛 수
    std::uint32_t drawn = 0;      // 이번에 그린 빛 수
    std::uint32_t offscreen = 0;  // 화면 밖이라 건너뛴 수
    std::uint32_t over_cap = 0;   // 한 화면 최대 개수를 넘어서 건너뛴 수
};

}  // namespace wind::engine
