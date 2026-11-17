#pragma once

// 이 파일은 무엇을 하나요?
// 몬스터 머리 위·발밑에 그리는 표시를 그리기 목록에 넣는 도우미 함수 모음이에요. (11b, 보여 주기 전용)
// - 느낌표 "!": 주인공을 알아챘을 때 머리 위에 톡 튀어 올라요. 조명 뒤에 그려서 밤에도 잘 보여요.
// - 공격 예고 바닥 표시: 덮칠 길을 따라 붉은 타원이 바닥에 깔리고, 예고가 끝날수록 커지고 진해져요.
//   몸은 번쩍이지 않아요 (개발자와 정함). 그림자처럼 몸 아래 바닥에 그려요.
// - 공격 예고 빛: 바닥 표시는 조명 앞에 그려져서 밤에는 어두워져요. 그래서 같은 자리에 붉은 빛을 하나 켜요.
//   (낮이나 조명 없는 맵에서는 빛이 아무 일도 안 해서 모양이 그대로예요)
// 값의 이유는 docs/VISUALS.md "Monsters" 절에 있어요.

#include "engine/camera/camera2d.hpp"
#include "engine/core/math.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"

namespace wind::game {

// 바닥 경고 표시 하나 (맵 위 픽셀)
struct TelegraphShape {
    engine::Vec2 from;          // 덮치기 시작 쪽 끝 (맵 위, 바닥)
    engine::Vec2 to;            // 덮치기 끝 쪽 끝
    float width = 14.0f;        // 길 폭 (픽셀, 공격 히트박스 폭)
};

// head: 머리 꼭대기의 맵 위 위치, age: 알아챈 뒤 흐른 시간 (초), alpha: 투명도 0~1
void queue_notice_mark(const engine::Camera2D& camera, engine::Vec2 head, float age, float alpha,
                       engine::RenderQueue& queue);

// glow: 가운데가 진한 흰 동그라미 그림 (빛 그림을 빌려 써요), progress: 예고 진행 0~1
// sort_y: 바닥 단계 안에서 줄 서는 기준 (몬스터 발 y)
void queue_telegraph(const engine::Camera2D& camera, engine::TextureHandle glow, const TelegraphShape& shape,
                     float progress, float sort_y, engine::RenderQueue& queue);

// 공격 예고 동안 바닥 표시 자리에 켜는 붉은 빛 (밤에도 경고가 보이게). progress: 예고 진행 0~1
[[nodiscard]] engine::Light telegraph_light(const TelegraphShape& shape, float progress);

}  // namespace wind::game
