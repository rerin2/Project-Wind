#pragma once

// 이 파일은 무엇을 하나요?
// 화면 왼쪽 아래 아이템 알림 줄의 "생각" 부분이에요. (14b, 개발자와 정함) 그리기는 app/loot_toast_view가 해요.
// - 주우면 "슬라임 젤리 ×2", 가방이 가득 차면 "가방이 가득 찼다 · 롱소드" (빨간 글자)
// - 줄은 왼쪽에서 미끄러져 들어와 2.6초 보이고 흐려지며 사라져요. 최대 4줄, 새 줄이 아래에 붙어요.
// - 같은 아이템을 1초 안에 또 주우면 새 줄 대신 개수를 더해요 ("×1" → "×3").
// 게임 규칙은 바꾸지 않고 보여 주기만 해요. SDL을 몰라요.

#include "game/items/loot_notice.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace wind::items {
class ItemCatalog;
}

namespace wind::game {

class LootToasts {
public:
    static constexpr std::size_t kMaxLines = 4;    // 한 번에 보이는 줄 수
    static constexpr float kLifeSeconds = 2.6f;    // 한 줄이 보이는 시간
    static constexpr float kMergeSeconds = 1.0f;   // 이 안에 같은 아이템을 또 주우면 개수를 더해요
    static constexpr float kSlideSeconds = 0.2f;   // 미끄러져 들어오는 시간
    static constexpr float kFadeSeconds = 0.4f;    // 사라지기 전 흐려지는 시간

    struct Line {
        std::string item_id;
        std::int32_t count = 0;
        std::string text;      // 보여 줄 글 ("슬라임 젤리 ×2")
        std::int32_t icon = 0; // 아이콘 칸 번호
        bool bag_full = false; // 빨간 경고 줄인가
        float age = 0.0f;      // 나타난 뒤 흐른 시간
    };

    // 알림 하나를 줄로 만들어요. 도감에서 이름·아이콘을 찾아요 (없으면 id 그대로)
    void push(const LootNotice& notice, const items::ItemCatalog& catalog);
    void update(float dt_seconds);

    [[nodiscard]] const std::vector<Line>& lines() const { return lines_; }
    // 미끄러져 들어온 정도 0~1, 투명도 0~1 (그리기용)
    [[nodiscard]] static float slide(const Line& line);
    [[nodiscard]] static float alpha(const Line& line);

private:
    std::vector<Line> lines_;  // 오래된 줄이 앞
};

}  // namespace wind::game
