// 이 파일은 무엇을 하나요?
// 필드 바꾸기: 출구·부활·쉬기 요청을 보고 MapTravel로 화면을 가린 뒤 새 필드를 만들어요.

#include "app/field_session.hpp"

#include "app/field_ui.hpp"
#include "app/save_keeper.hpp"
#include "game/progression/hero_progress.hpp"
#include "game/save/wake_place.hpp"
#include "game/states/field_state.hpp"
#include "game/world/travel_targets.hpp"

#include "wind/log.hpp"

#include <utility>

namespace wind::client {
namespace {

// 출구로 맵을 옮길 때 화면이 어두워지는(또는 밝아지는) 시간 (초)
constexpr float kFadeSeconds = 0.35f;
// 부활할 때의 느린 페이드 (어두워지기·밝아지기 각각, 12c, 개발자와 정함): "깨어나는" 느낌
constexpr float kRespawnFadeSeconds = 0.8f;
// 쉬기 페이드 (13b, 개발자와 정함: "잠깐 어두워짐"). 그동안 맵을 새로 읽어 몬스터를 다시 놓아요.
constexpr float kRestFadeSeconds = 0.5f;

}  // namespace

FieldSession::FieldSession(MakeField make_field, game::HeroProgress& progress, FieldUi& ui, SaveKeeper& saver,
                           std::filesystem::path content_root)
    : make_field_(std::move(make_field)),
      progress_(progress),
      ui_(ui),
      saver_(saver),
      content_root_(std::move(content_root)),
      travel_(kFadeSeconds) {}

FieldSession::~FieldSession() = default;  // FieldState는 .cpp에서만 완전히 알아요

bool FieldSession::start(const std::string& map, std::optional<engine::Vec2> feet) {
    field_ = make_field_(map, game::kDefaultSpawn, feet);
    if (!field_) return false;
    current_map_ = map;
    ui_.enter_area(field_->display_name());  // 첫 지역 이름 배너
    return true;
}

void FieldSession::update(float frame_seconds, bool fallen_ready, bool confirm) {
    // 방금 쓰러져 룬을 잃었나요? 그 박자에 바로 저장해요 (강제 종료로 룬을 되살리지 못하게, 명세 12.3)
    if (field_->take_hero_fell()) save_after_fall();
    // 부활할 때인가요? 쓰러짐 화면이 다 나타난 뒤 Enter 또는 2.5초 (맵을 옮기는 중에는 세지 않아요, 12c)
    const bool respawn_now = respawn_.update(frame_seconds, field_->hero_down(), fallen_ready && !busy(), confirm);
    if (!busy()) {
        if (auto exit = field_->take_exit_request()) {
            travel_.begin(std::move(*exit));  // 출구: 기본 페이드(또는 아이리스)
        } else if (auto cushion = field_->take_rest_request()) {
            // 쉬기: 잠깐 어두워졌다가 같은 맵을 새로 읽어요 (몬스터 다시 나타남, 명세 8.7 쉬기 4번)
            travel_.begin(game::cushion_target(current_map_, *cushion), {game::TravelKind::Rest, kRestFadeSeconds});
        } else if (respawn_now) {
            // 부활: 느린 페이드 -> 마지막 쿠션(없으면 지금 맵의 기본 자리)에서 가득 찬 채 깨어나요 (명세 9.5)
            // 저장은 쓰러진 박자에 이미 했어요 (같은 자리·같은 상태)
            travel_.begin(game::respawn_target(progress_, current_map_),
                          {game::TravelKind::Respawn, kRespawnFadeSeconds});
        }
    }
    travel_.update(frame_seconds, [this](const world::MapExit& exit, game::TravelKind kind) {
        return enter(exit, kind);
    });
}

bool FieldSession::save_at_cushion() {
    return save_or_warn(save::SaveKind::Main, hero_place());  // 앉아 있는 쿠션 자리 그대로
}

game::HeroPlace FieldSession::hero_place() const {
    const auto vitals = field_->hero_vitals();
    return {current_map_, field_->hero_feet(), vitals.hp, vitals.mana};
}

bool FieldSession::save_or_warn(save::SaveKind kind, const game::HeroPlace& place) {
    if (saver_.save(kind, place)) return true;
    ui_.announce("저장하지 못했다");  // 게임은 그대로 계속해요 (이유는 SaveKeeper가 로그에)
    return false;
}

void FieldSession::save_after_fall() {
    // 깨어날 자리: 마지막 쿠션(없으면 지금 맵 기본 자리), HP·MP 가득 (2026-11-24 개발자와 정함)
    const auto& hp = field_->health();
    const auto& mp = field_->mana();
    const auto place = game::wake_place(content_root_, progress_, current_map_, hp.max(), mp.max());
    if (!place) {
        ui_.announce("저장하지 못했다");  // 맵 파일이 틀렸어요 (이유는 wake_place가 로그에)
        return;
    }
    // 본과 자동 둘 다 (자동 저장을 불러와 룬을 되살리지 못하게). 하나가 실패해도 다른 하나는 해 봐요
    const bool main_saved = saver_.save(save::SaveKind::Main, *place);
    const bool auto_saved = saver_.save(save::SaveKind::Auto, *place);
    if (!main_saved || !auto_saved) ui_.announce("저장하지 못했다");
}

bool FieldSession::enter(const world::MapExit& exit, game::TravelKind kind) {
    const game::HeroVitals carried = field_->hero_vitals();  // 떠나기 전 값 (새 필드를 만들기 전에)
    auto next = make_field_(exit.to_map, exit.to_spawn, std::nullopt);
    if (!next) return false;  // 지금 필드는 그대로 (MapTravel이 알려 줘요)
    if (kind == game::TravelKind::Exit) {
        next->restore_hero(carried);  // 맵을 오가도 HP가 다시 차지 않아요 (12c)
        log::info("Hero carries hp {}, mp {:.0f}, stamina {:.0f} into the new map", carried.hp, carried.mana,
                  carried.stamina);
    }
    current_map_ = exit.to_map;
    field_ = std::move(next);
    if (kind == game::TravelKind::Rest) {
        // 부활 자리를 이 쿠션으로
        const bool first = progress_.rest_at(current_map_, exit.to_spawn);
        if (first) {  // 처음 찾음: 쿠션이 피어나고 배너가 떠요
            field_->bloom_cushion(exit.to_spawn);
        }
        // 명세 8.7 쉬기 5번: 회복된 채 쿠션 자리에서 지금 슬롯에 자동 저장 (16b).
        // 실패해도 쉬기는 그대로 하고 알려요 (실패 배너가 처음 찾은 쿠션 배너보다 먼저)
        if (save_or_warn(save::SaveKind::Main, hero_place()) && first) {
            ui_.announce("캣닢 쿠션을 찾았다 — " + field_->cushion_name(exit.to_spawn));
        }
        menu_request_ = true;  // 명세 8.7 쉬기 6번: 쿠션 메뉴를 열어요 (13c)
        return true;  // 같은 맵이라 지역 이름 배너는 띄우지 않아요
    }
    ui_.enter_area(field_->display_name());  // 새 지역 이름 배너 (출구·부활)
    // 지역 전환 (16c, 명세 12.3): 새 지역 입구에 선 지금을 자동 슬롯에만 (갑자기 꺼졌을 때 대비).
    // 부활은 쓰러진 박자에 이미 저장했어요
    if (kind == game::TravelKind::Exit) (void)save_or_warn(save::SaveKind::Auto, hero_place());
    return true;
}

}  // namespace wind::client
