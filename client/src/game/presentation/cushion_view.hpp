#pragma once

// 이 파일은 무엇을 하나요?
// 캣닢 쿠션 연출이에요. (명세 8.7, 13b) 게임 결과는 바꾸지 않고 보여 주기만 해요.
// - 쿠션 그림: 아직 못 찾은 쿠션은 시든 모습, 찾은 쿠션은 피어난 모습
// - 처음 찾으면: 퐁 하고 커졌다 돌아오며 시든 모습에서 피어나고, 꽃잎 파티클이 터지고, 빛이 번쩍해요
// - 레벨업(13c): 주인공 발에서 금빛 알갱이가 기둥처럼 솟고 금빛이 번쩍해요
// - 쉬는 동안: 쿠션 둘레에 초록 빛이 숨 쉬듯 밝아졌다 어두워지고,
//   앉는 순간 하트 두 개가 퐁, 웅크려 졸면 "z"가 머리 위로 둥실 떠올라요
// 그림 판은 content/props/catnip_cushion.bmp (칸 32x32: 시든·피어난·z·하트).

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_effect.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/entities/hero_rest.hpp"
#include "game/world/cushion.hpp"

#include <functional>
#include <string_view>
#include <vector>

namespace wind::game {

// 쿠션 그림 판 위치 (콘텐츠 폴더 기준)
inline constexpr std::string_view kCushionTexture = "props/catnip_cushion.bmp";

class CushionView {
public:
    // cushions: 맵의 쿠션들, discovered: 쿠션 id를 이미 찾았나 묻는 함수, texture: 쿠션 그림 판,
    // bloom_fx: 처음 찾았을 때 터지는 꽃잎 효과, level_up_fx: 레벨업 빛기둥 효과 (없으면 nullptr, 13c)
    CushionView(std::vector<Cushion> cushions, const std::function<bool(std::string_view)>& discovered,
                engine::TextureHandle texture,
                const engine::ParticleEffect* bloom_fx, const engine::ParticleEffect* level_up_fx = nullptr);

    // 처음 찾았어요: 이 쿠션을 피어나게 해요 (이미 피었으면 아무 일 없음)
    void bloom(std::string_view cushion_id);
    // 레벨업했어요 (13c): feet(주인공 발)에서 0.5초 동안 빛 알갱이가 위로 솟고, 금빛이 번쩍해요
    void celebrate_level_up(engine::Vec2 feet);
    // 화면 한 장마다. resting: 주인공이 쉬고 있는 쿠션 (없으면 nullptr), rest: 쉬기 시간표
    void update(float dt_seconds, const Cushion* resting, const HeroRest& rest, engine::ParticleRandom& random,
                engine::ParticlePool& air_fx);
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const;
    // 쿠션 빛 (쉬는 동안 초록 빛, 피어나는 순간 번쩍)
    [[nodiscard]] std::vector<engine::Light> lights() const;

    // 테스트용: 떠 있는 z·하트 수, 쿠션이 피었나
    [[nodiscard]] std::size_t floater_count() const { return floaters_.size(); }
    [[nodiscard]] bool bloomed(std::size_t index) const { return looks_[index].bloomed; }

private:
    enum class FloaterKind : std::uint8_t { Z, Heart };
    struct Floater {
        FloaterKind kind;
        engine::Vec2 start;  // 처음 자리 (맵 위 픽셀)
        float age = 0.0f;    // 지난 시간
        float drift = 0.0f;  // 옆으로 흔들리는 방향·크기 (-1~1)
    };
    struct Look {
        bool bloomed = false;    // 피어난 모습인가
        float bloom_age = -1.0f; // 피어나기 연출이 시작된 뒤 시간 (음수면 연출 없음)
    };

    void spawn(FloaterKind kind, engine::Vec2 seat, float drift);
    void queue_floater(const engine::Camera2D& camera, engine::RenderQueue& queue, const Floater& floater) const;

    std::vector<Cushion> cushions_;
    std::vector<Look> looks_;              // cushions_와 같은 순서
    engine::TextureHandle texture_;
    const engine::ParticleEffect* bloom_fx_;
    std::vector<Floater> floaters_;        // 떠오르는 z·하트
    const Cushion* resting_ = nullptr;     // 지금 쉬는 쿠션 (빛 자리)
    HeroRest::Phase last_phase_ = HeroRest::Phase::None;
    float z_timer_ = 0.0f;                 // 다음 z까지 남은 시간
    float glow_clock_ = 0.0f;              // 쉬는 동안 흐른 시간 (빛이 서서히 켜져요)
    std::vector<std::size_t> burst_queue_; // 다음 update에서 꽃잎을 터뜨릴 쿠션 번호 (bloom에는 주사위가 없어서)
    const engine::ParticleEffect* level_up_fx_;  // 레벨업 빛기둥 (13c)
    float level_up_age_ = -1.0f;            // 레벨업 연출이 시작된 뒤 시간 (음수면 없음)
    float level_up_emit_ = 0.0f;            // 다음 빛 알갱이 묶음까지 남은 시간
    engine::Vec2 level_up_at_{};            // 레벨업한 자리 (발)
};

}  // namespace wind::game
