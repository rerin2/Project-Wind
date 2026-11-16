#pragma once

// 이 파일은 무엇을 하나요?
// 싸움 화면 담당이에요. (9단계 9c) 허수아비를 "보여 주기만" 해요.
// - 허수아비 그림: 서 있으면 0번 칸, 쓰러지면 1번 칸. 발밑 그림자도 그려요.
// - 머리 위 작은 HP 바: 맞으면 바로 줄고 꼬리가 남았다가 따라 줄어요 (지연 바).
//   HP가 가득 찬 채로 한동안 있으면 사라지고, 맞으면 다시 보여요.
// - F4: 칼 히트박스(빨강, 판정 중일 때만)와 허수아비 허트박스(노랑)를 반투명 네모로 보여 줘요.
// - 맞으면 0.08초 동안 하얗게 번쩍여요 (10b). 같은 그림을 "더하기 섞기"로 한 번 더 그려서 밝게 해요.
//   번쩍임 세기는 접근성 설정(--flash)을 따라요. 0이면 번쩍이지 않아요.
// 게임 규칙(HP, 판정)은 바꾸지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/render/render_queue.hpp"
#include "game/combat/training_dummy.hpp"
#include "game/ui/delayed_bar.hpp"

#include <optional>
#include <span>
#include <vector>

namespace wind::game {

class CombatView {
public:
    // shadow: 발밑 그림자 그림, dummy_textures: 허수아비마다 그림 (dummies와 같은 순서)
    CombatView(engine::TextureHandle shadow, std::vector<engine::TextureHandle> dummy_textures);

    // 화면 한 장마다: HP 바를 허수아비 HP에 맞추고, 번쩍임 시간을 줄여요.
    void update(float dt_seconds, std::span<const TrainingDummy> dummies);
    // target 번호의 허수아비를 번쩍이게 해요 (맞은 일마다)
    void flash(std::span<const TrainingDummy> dummies, TargetId target);
    // 번쩍임 세기 0~1 (접근성)
    void set_flash_strength(float strength);
    // index번 허수아비의 지금 번쩍임 0~1 (세기 설정을 곱한 값)
    [[nodiscard]] float flash_amount(std::size_t index) const;
    // 허수아비·그림자·HP 바를 그리기 목록에 넣어요. show_boxes(F4)면 판정 상자도.
    void queue(const engine::Camera2D& camera, std::span<const TrainingDummy> dummies,
               const std::optional<engine::RectF>& hitbox, bool show_boxes,
               engine::RenderQueue& queue) const;

private:
    void queue_dummy(const engine::Camera2D& camera, const TrainingDummy& dummy, std::size_t index,
                     engine::RenderQueue& queue) const;
    void queue_bar(const engine::Camera2D& camera, const TrainingDummy& dummy, const DelayedBar& bar,
                   engine::RenderQueue& queue) const;

    engine::TextureHandle shadow_;                  // 발밑 그림자
    std::vector<engine::TextureHandle> textures_;   // 허수아비 그림 (순서대로)
    std::vector<DelayedBar> bars_;                  // 허수아비 HP 바 (순서대로)
    std::vector<float> flash_left_;                 // 허수아비마다 남은 번쩍임 시간 (초)
    float flash_strength_ = 1.0f;                   // 번쩍임 세기 (접근성)
};

}  // namespace wind::game
