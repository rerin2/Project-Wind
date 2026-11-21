#pragma once

// 이 파일은 무엇을 하나요?
// "쓰러졌다" 화면의 시간표예요. (명세 9.5, 13.1, 12b)
// 주인공이 쓰러지면
// 1) 1.2초 기다리고 (슬로모션·흑백이 먼저 보이게)
// 2) 0.8초 동안 글자와 띠가 아래에서 살짝 올라오며 나타나고
// 3) 다 나타나면 "바로 부활" 안내가 0.4초 동안 떠올라요. 이때부터 부활할 수 있어요(ready, 12c: RespawnTrigger가 봐요).
// 다시 서면(새 맵) 바로 사라져요.
// 그리지는 않고 투명도·위치만 계산해요 (그리기는 app/field_ui). 게임 규칙에는 영향이 없어요.
// 값의 이유는 docs/VISUALS.md "Hero falling" 절에 있어요.

namespace wind::game {

class FallenScreen {
public:
    // 화면 한 장마다. down: 주인공이 쓰러져 있나
    void update(float dt_seconds, bool down);

    [[nodiscard]] bool visible() const { return alpha() > 0.0f; }
    // 글자·띠 투명도 0~1
    [[nodiscard]] float alpha() const;
    // 아래로 몇 픽셀 내려가 있는지 (나타날 때 6 -> 0)
    [[nodiscard]] float drop() const;
    // "바로 부활" 안내 투명도 0~1
    [[nodiscard]] float hint_alpha() const;
    // 다 나타나서 부활해도 되나요? (12c)
    [[nodiscard]] bool ready() const;

    static constexpr float kDelaySeconds = 1.2f;
    static constexpr float kInSeconds = 0.8f;
    static constexpr float kHintSeconds = 0.4f;
    static constexpr float kDropPixels = 6.0f;

private:
    bool down_ = false;
    float seconds_ = 0.0f;  // 쓰러진 뒤 흐른 화면 시간
};

}  // namespace wind::game
