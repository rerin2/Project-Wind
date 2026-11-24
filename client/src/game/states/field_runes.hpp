#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면의 룬 담당이에요. (13a 룬 받기·잃기, 14e 룬 흔적) FieldState가 300줄을 넘지 않게 여기 모았어요.
// 정해진 박자마다 (규칙):
//   1) 이번 박자에 쓰러뜨린 몬스터의 룬을 지갑에 넣어요.
//   2) 주인공이 쓰러진 박자면 룬 절반을 잃고, 쓰러진 자리에 룬 흔적이 생겨요 (이전 흔적은 사라져요).
//      같은 박자에 몬스터도 쓰러뜨렸으면 룬을 먼저 받고 그다음 절반을 잃어요 (13a와 같아요).
//   3) 이 맵에 흔적이 있고, 주인공이 살아 있고 쉬는 중이 아니며, 발이 흔적에서 24픽셀 안이면 전부 되찾아요.
// 화면 한 장마다 (연출): 흔적 웅덩이·가루·빛, 되찾으면 알갱이가 빨려 들어요 (RuneTraceView).
// 흔적은 HeroProgress가 가지고 있어서 맵을 옮기거나 부활해도 남아요.

#include "engine/camera/camera2d.hpp"
#include "engine/fx/particle_pool.hpp"
#include "engine/fx/particle_random.hpp"
#include "engine/light/light.hpp"
#include "engine/render/render_queue.hpp"
#include "game/presentation/rune_trace_view.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wind::game {

class FieldCombat;
class HeroProgress;
class PlayerMotion;

class FieldRunes {
public:
    static constexpr float kRecoverRadius = 24.0f;  // 발이 흔적에서 이 안이면 되찾아요 (픽셀, 명세 8.3.1)

    // progress: 주인공 진행 상태 (앱이 가지고 있어요), map: 이 맵 파일, glow·motes: 흔적 연출 그림·가루
    FieldRunes(HeroProgress& progress, std::string_view map, engine::TextureHandle glow,
               const engine::ParticleEffect* motes);

    // 정해진 박자마다: 룬 받기 -> 쓰러지면 잃기와 흔적 -> 흔적 되찾기
    void step(const FieldCombat& combat, const PlayerMotion& hero);
    // 지난번 뒤로 흔적을 되찾았으면 그 양을 한 번 돌려줘요 ("룬을 되찾았다" 배너용)
    [[nodiscard]] std::optional<std::int64_t> take_recovered() { return std::exchange(recovered_, std::nullopt); }
    // 지난번 뒤로 쓰러져 룬을 잃었으면 한 번 true (그 박자에 저장해요, 16c)
    [[nodiscard]] bool take_fell() { return std::exchange(fell_, false); }

    // 화면 한 장마다 (연출). feet: 주인공 발 (그린 위치)
    void update(float dt_seconds, engine::Vec2 feet, engine::ParticleRandom& random, engine::ParticlePool& air_fx);
    void queue(const engine::Camera2D& camera, engine::RenderQueue& queue) const { view_.queue(camera, queue); }
    [[nodiscard]] std::vector<engine::Light> lights() const { return view_.lights(); }

private:
    // 이 맵에 흔적이 있으면 그 자리
    [[nodiscard]] std::optional<engine::Vec2> trace_here() const;

    HeroProgress& progress_;
    std::string map_;                            // 이 맵 파일 (흔적이 이 맵 것인지 볼 때)
    RuneTraceView view_;
    std::optional<std::int64_t> recovered_;      // 배너용 (앱이 가져가요)
    bool fell_ = false;                          // 방금 쓰러져 룬을 잃었어요 (저장용, 앱이 가져가요, 16c)
    std::optional<std::pair<engine::Vec2, std::int64_t>> recovered_fx_;  // 다음 화면에 알갱이를 튀길 자리와 양
};

}  // namespace wind::game
