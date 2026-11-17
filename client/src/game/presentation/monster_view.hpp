#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터를 "보여 주기만" 하는 담당이에요. (11a, 11b)
// - 발밑 그림자 (몸이 뜨면 조금 작아져요)
// - 몬스터 그림: 쉴 때 숨쉬기, 걸을 때 통통 뛰기, 싸움 동작(폴짝·웅크림·덮치기·철퍽) (monster_pose)
// - 알아챘을 때 머리 위 느낌표, 공격 예고 동안 바닥의 붉은 경고 표시와 붉은 빛 (monster_marks)
// - 맞으면 하얗게 번쩍, 머리 위 HP 바(허수아비와 같은 지연 바), 쓰러지면 깜빡이며 녹아내리고 사라져요 (11d)
// - F4: 맞는 상자(노랑), 발 상자(하늘색), 덮치는 동안 공격 판정(빨강)
// 위치는 두 박자 사이를 이어서(보간) 그려서 부드러워요. 게임 규칙(위치·상태)은 바꾸지 않아요.

#include "engine/camera/camera2d.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/monsters/monster.hpp"
#include "game/presentation/monster_marks.hpp"
#include "game/ui/delayed_bar.hpp"

#include <optional>
#include <span>
#include <vector>

namespace wind::game {

class MonsterView {
public:
    // shadow: 발밑 그림자 그림, glow: 바닥 경고 표시에 쓰는 둥근 빛 그림,
    // textures: 몬스터마다 그림 (monsters와 같은 순서)
    MonsterView(engine::TextureHandle shadow, engine::TextureHandle glow, std::vector<engine::TextureHandle> textures);

    // target 번호의 몬스터를 번쩍이게 해요 (맞은 일마다, 11d)
    void flash(std::span<const Monster> monsters, TargetId target);
    // 번쩍임 세기 0~1 (접근성 --flash). 쓰러질 때 깜빡임에도 곱해요
    void set_flash_strength(float strength);
    // 화면 한 장마다: 연출 시계, 마리마다 뜀 세기·느낌표 나이·HP 바·번쩍임을 흘려요
    void update(float dt_seconds, std::span<const Monster> monsters);
    // 바닥 경고·그림자·몬스터·느낌표를 그리기 목록에 넣어요. show_boxes(F4)면 상자도.
    void queue(const engine::Camera2D& camera, std::span<const Monster> monsters, float alpha, bool show_boxes,
               engine::RenderQueue& queue) const;
    // 조명에 더할 빛: 공격 예고 중인 몬스터마다 바닥 표시 자리에 붉은 빛 (밤에도 경고가 보이게)
    [[nodiscard]] std::vector<engine::Light> lights(std::span<const Monster> monsters) const;

private:
    void queue_monster(const engine::Camera2D& camera, const Monster& monster, std::size_t index, float alpha,
                       engine::RenderQueue& queue) const;
    // 지금 하얀 번쩍임 0~1: 맞은 순간 + 쓰러지는 동안 깜빡임, 세기 설정을 곱한 값
    [[nodiscard]] float flash_amount(const Monster& monster, std::size_t index) const;
    // 예고 중이면 바닥 표시 모양 (덮칠 길), 아니면 없음
    [[nodiscard]] static std::optional<TelegraphShape> telegraph_shape(const Monster& monster);
    // 공격 예고 중이면 덮칠 길을 바닥에 붉게 (판정과 같은 "지금 박자" 위치)
    void queue_telegraph_mark(const engine::Camera2D& camera, const Monster& monster,
                              engine::RenderQueue& queue) const;
    // F4 상자는 판정과 같은 "지금 박자" 위치에 그려요 (보간하지 않아요)
    void queue_boxes(const engine::Camera2D& camera, const Monster& monster,
                     engine::RenderQueue& queue) const;

    engine::TextureHandle shadow_;                 // 발밑 그림자
    engine::TextureHandle glow_;                   // 둥근 빛 그림 (바닥 경고 표시)
    std::vector<engine::TextureHandle> textures_;  // 몬스터 그림 (순서대로)
    std::vector<float> hop_blend_;                 // 마리마다 뜀 세기 0~1
    std::vector<float> notice_age_;                // 마리마다 느낌표가 뜬 뒤 흐른 시간 (초)
    std::vector<DelayedBar> bars_;                 // 마리마다 머리 위 HP 바 (11d)
    std::vector<float> flash_left_;                // 마리마다 남은 번쩍임 시간 (초, 11d)
    float flash_strength_ = 1.0f;                  // 번쩍임 세기 (접근성)
    float clock_ = 0.0f;                           // 숨쉬기·떨림 시계 (초)
};

}  // namespace wind::game
