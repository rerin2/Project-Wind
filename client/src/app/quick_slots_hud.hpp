#pragma once

// 이 파일은 무엇을 하나요?
// 화면 아래 가운데의 단축 아이템 칸 1~4예요. (명세 13.2, 15f, 개발자와 정함: 아래 가운데)
// - 칸 네 개 (24픽셀, 왼쪽 아래 무기·마술 칸과 같은 크기·색). 칸마다 왼쪽 위에 번호(진한 금빛).
// - 등록된 칸: 아이콘, 오른쪽 아래에 가방 속 개수. 0개면 아이콘을 흐리게, 개수 "0"은 빨갛게 (등록은 남아요).
// - 빈 칸: 번호와 테두리만 흐리게.
// - 개수가 줄면(썼어요) 그 칸이 0.18초 동안 톡 커졌다 돌아와요.
// 보여 주기 전용이에요. 값은 ItemHud가 단축 칸·가방·도감에서 모아 줘요.

#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace wind::engine {
class UiPainter;
}

namespace wind::client {

// 단축 칸 하나가 보여 줄 것
struct QuickSlotInfo {
    bool assigned = false;     // 아이템이 등록돼 있나
    int icon = 0;              // 아이템 아이콘 칸
    std::int64_t count = 0;    // 가방 속 개수
};

class QuickSlotsHud {
public:
    static constexpr std::size_t kCount = 4;

    explicit QuickSlotsHud(engine::TextureHandle icons) : icons_(icons) {}

    // 화면 한 장마다: 개수가 줄어든 칸은 톡
    void update(float dt_seconds, const std::array<QuickSlotInfo, kCount>& info);
    void draw(engine::UiPainter& painter, engine::Vec2 screen) const;

private:
    engine::TextureHandle icons_;
    std::array<QuickSlotInfo, kCount> info_{};
    std::array<std::optional<std::int64_t>, kCount> seen_count_{};  // 지난 장의 개수 (처음엔 없음)
    std::array<float, kCount> pop_{};                               // 톡 남은 시간
};

}  // namespace wind::client
