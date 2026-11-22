#pragma once

// 이 파일은 무엇을 하나요?
// 필드의 캣닢 쿠션 담당이에요. (명세 8.7, 13b)
// - 맵의 쿠션들을 가지고 있어요.
// - E를 누르면(press_interact) 다음 박자에:
//     앉아 있으면 -> 기지개 켜고 일어나기
//     아니면 쉬기 조건(rest_rules)을 보고
//       쉴 수 있으면 -> 주인공이 쿠션에 앉기 시작 (HP·MP·스태미나 가득, PlayerMotion::begin_rest)
//       소란스러우면 -> "쉴 수 없음" 횟수를 올려요 (UI가 알림을 띄워요)
// - 앉고 나서 잠깐(0.6초) 뒤 "쉬기 이동" 요청을 남겨요. 앱이 페이드 -> 같은 맵을 새로 읽어
//   몬스터를 다시 놓고, 주인공은 쿠션 위에 앉은 채 시작해요 (부활 자리 정하기·자동 저장 자리도 그때).
// 게임 규칙이라 SDL을 몰라요.

#include "game/world/cushion.hpp"
#include "game/world/rest_rules.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace wind::game {

class FieldCombat;
class PlayerMotion;

// 앉고 나서 쉬기 이동(페이드)을 시작할 때까지 기다리는 시간 (초). 앉는 모습을 잠깐 보여 줘요.
inline constexpr float kRestTravelDelaySeconds = 0.6f;

class FieldRest {
public:
    explicit FieldRest(std::vector<Cushion> cushions, RestRules rules = {});

    // E를 눌렀어요 (화면 한 장마다, 박자 전에). 다음 박자에 처리해요.
    void press_interact() { interact_ = true; }
    // 한 박자 (주인공·싸움 계산 뒤에 불러요)
    void step(float dt_seconds, PlayerMotion& player, const FieldCombat& combat);
    // 이 쿠션 위에서 앉은 채 시작해요 (새 맵을 쉬기·부활로 열었을 때). 그 쿠션이 없으면 false.
    bool sit_on(std::string_view cushion_id, PlayerMotion& player);

    // 쉬기 이동 요청을 꺼내요 (한 번만). 쿠션 id
    [[nodiscard]] std::optional<std::string> take_travel_request();
    // "[E] 쉬기" 안내를 보여 줄 쿠션 (가까이 서 있고 쉴 수 있는 상태일 때), 없으면 nullptr
    [[nodiscard]] const Cushion* prompt_cushion(const PlayerMotion& player) const;
    // 주인공이 쉬고 있는 쿠션 (초록 빛·z 연출용), 없으면 nullptr
    [[nodiscard]] const Cushion* resting_cushion() const;
    // 소란스러워서 못 쉰 횟수 (늘기만 해요. UI가 바뀐 걸 보고 알림을 띄워요)
    [[nodiscard]] unsigned denied_count() const { return denied_count_; }
    [[nodiscard]] const std::vector<Cushion>& cushions() const { return cushions_; }
    // 이 맵에 있는 쿠션의 이름 (배너용). 없는 id면 빈 글자
    [[nodiscard]] std::string name_of(std::string_view cushion_id) const;

private:
    void interact(PlayerMotion& player, const FieldCombat& combat);

    std::vector<Cushion> cushions_;            // 맵의 쿠션들
    RestRules rules_;                          // 24픽셀, 3초
    bool interact_ = false;                    // 이번 박자에 처리할 E
    std::optional<std::size_t> resting_on_;    // 쉬고 있는 쿠션 번호
    float travel_wait_ = -1.0f;                // 쉬기 이동까지 남은 시간 (음수면 기다리지 않음)
    std::optional<std::string> travel_request_;  // 앱이 꺼내 갈 쉬기 이동 요청
    unsigned denied_count_ = 0;
};

}  // namespace wind::game
