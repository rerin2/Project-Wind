#pragma once

// 이 파일은 무엇을 하나요?
// 싸움 화면 담당이에요. (9단계 9c) 허수아비를 "보여 주기만" 해요.
// - 허수아비 그림: 서 있으면 0번 칸, 쓰러지면 1번 칸. 발밑 그림자도 그려요.
// - 머리 위 작은 HP 바: 맞으면 바로 줄고 꼬리가 남았다가 따라 줄어요 (지연 바).
//   HP가 가득 찬 채로 한동안 있으면 사라지고, 맞으면 다시 보여요.
// - F4: 칼 히트박스(빨강, 판정 중일 때만)와 허수아비 허트박스(노랑)를 반투명 네모로 보여 줘요.
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

    // 화면 한 장마다: HP 바를 허수아비 HP에 맞춰요.
    void update(float dt_seconds, std::span<const TrainingDummy> dummies);
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
};

}  // namespace wind::game
