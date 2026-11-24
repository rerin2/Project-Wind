#pragma once

// 이 파일은 무엇을 하나요?
// 지금 필드 화면 하나와 "다른 필드로 바꾸는 일"을 맡아요. (7단계 출구, 12c 부활, 13b 쉬기)
// ClientApp에서 맵 옮기기 부분을 떼어 왔어요(파일이 300줄을 넘어서, 동작은 같아요).
// - 출구를 밟으면: 페이드(또는 아이리스) -> 새 맵. HP·MP·스태미나를 들고 가요.
// - 쓰러진 뒤 Enter 또는 2.5초: 느린 페이드 -> 마지막 쿠션(없으면 지금 맵 기본 자리). 가득 찬 채 깨어나요.
// - 쿠션에 앉은 지 잠깐 지나면: 페이드 -> 같은 맵을 새로 읽어 몬스터가 다시 나오고, 쿠션에 앉은 채 시작해요.
//   부활 자리를 이 쿠션으로 정하고, 처음 찾은 쿠션이면 배너를 띄우고, 지금 슬롯에 자동 저장해요 (16b).
// - 저장 (16c): 쓰러진 박자에 "깨어난 뒤 상태"를 본+자동에, 출구로 새 지역에 들어가면 자동에,
//   쿠션 메뉴 "저장"이면 본에 저장해요. 실패하면 "저장하지 못했다" 배너.
// 새 맵을 못 열면 지금 필드는 그대로 두고 화면에 잠깐 알려 줘요 (MapTravel).

#include "engine/core/math.hpp"
#include "game/save/save_sources.hpp"
#include "game/world/map_travel.hpp"
#include "game/world/respawn_trigger.hpp"

#include "wind/save/save_slots.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace wind::game {
class FieldState;
class HeroProgress;
}  // namespace wind::game

namespace wind::client {

class FieldUi;
class SaveKeeper;

class FieldSession {
public:
    // 맵 하나로 필드를 만드는 방법 (못 만들면 nullptr)
    // start: 저장된 발 위치 (불러온 첫 필드만, 16b). 비면 spawn 자리
    using MakeField = std::function<std::unique_ptr<game::FieldState>(std::string_view map, std::string_view spawn,
                                                                        std::optional<engine::Vec2> start)>;

    // saver: 저장 담당 (16b·16c), content_root: 깨어날 자리를 찾을 때 맵 파일을 읽는 곳 (16c)
    FieldSession(MakeField make_field, game::HeroProgress& progress, FieldUi& ui, SaveKeeper& saver,
                 std::filesystem::path content_root);
    ~FieldSession();
    FieldSession(const FieldSession&) = delete;
    FieldSession& operator=(const FieldSession&) = delete;

    // 첫 맵을 열어요 ("default" 자리, 저장을 불러왔으면 그 발 위치). 못 열면 false (게임을 시작할 수 없어요)
    bool start(const std::string& map, std::optional<engine::Vec2> feet = {});
    [[nodiscard]] game::FieldState& field() { return *field_; }
    [[nodiscard]] const game::FieldState& field() const { return *field_; }

    // 화면 한 장마다 (필드 frame_update 뒤): 부활할 때인지, 출구·쉬기 요청이 있는지 보고 옮기기를 진행해요.
    // fallen_ready: "쓰러졌다" 화면이 다 나타났나, confirm: Enter를 눌렀나
    void update(float frame_seconds, bool fallen_ready, bool confirm);

    // 옮기는 중인가요? (게임 입력을 막아요)
    [[nodiscard]] bool busy() const { return travel_.busy(); }
    [[nodiscard]] const engine::ScreenFade& fade() const { return travel_.fade(); }
    [[nodiscard]] const std::string& notice() const { return travel_.notice(); }
    // 쉬기 이동이 끝나 쿠션 메뉴를 열 때인가요? (한 번만 true, 13c. 부활로 깨어날 때는 열지 않아요)
    [[nodiscard]] bool take_menu_request() { return std::exchange(menu_request_, false); }
    // 쿠션 메뉴 "저장" (16c): 쿠션에 앉은 지금 자리를 본 슬롯에. 성공하면 true (실패하면 배너도)
    bool save_at_cushion();
    // 필드를 정리해요 (게임을 끝낼 때, 그림 관리자보다 먼저)
    void shutdown() { field_.reset(); }

private:
    // 화면이 다 가려졌을 때 새 맵을 열어요. 못 열면 false
    bool enter(const world::MapExit& exit, game::TravelKind kind);
    // 지금 필드의 주인공 자리·HP·MP
    [[nodiscard]] game::HeroPlace hero_place() const;
    // 저장하고, 실패하면 "저장하지 못했다" 배너 (이유는 SaveKeeper가 로그에). 성공하면 true
    bool save_or_warn(save::SaveKind kind, const game::HeroPlace& place);
    // 쓰러진 박자: 깨어날 자리·가득 찬 HP·MP로 본과 자동 둘 다 (명세 12.3, 16c)
    void save_after_fall();

    MakeField make_field_;
    game::HeroProgress& progress_;   // 부활 자리·찾은 쿠션 (앱이 가지고 있어요)
    FieldUi& ui_;                    // 지역 이름·쿠션 배너
    SaveKeeper& saver_;              // 저장 담당 (16b·16c)
    std::filesystem::path content_root_;  // 깨어날 맵 파일을 읽는 곳 (16c)
    std::unique_ptr<game::FieldState> field_;
    game::MapTravel travel_;         // 화면 가리기 -> 새 맵 -> 보여 주기
    game::RespawnTrigger respawn_;   // 쓰러진 뒤 언제 부활할지 (Enter 또는 2.5초, 12c)
    std::string current_map_;        // 지금 맵 파일 (부활·쉬기에 써요)
    bool menu_request_ = false;      // 쿠션 메뉴를 열어 달라는 요청 (13c)
};

}  // namespace wind::client
