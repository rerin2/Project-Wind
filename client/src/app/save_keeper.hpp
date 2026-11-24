#pragma once

// 이 파일은 무엇을 하나요?
// 지금 슬롯에 게임을 저장하는 담당이에요. (명세 12.3, 16b)
// - 저장 폴더·슬롯 번호와 플레이 시간을 들고 있어요.
// - save(종류, 주인공 자리): 게임 상태를 모아(capture_save) 저장한 때·플레이 시간·게임 버전을 붙이고
//   SaveSlots로 안전하게 써요. 실패하면 이유를 로그에 남기고 false (게임은 그대로 계속해요).
// 언제 저장할지는 부르는 쪽이 정해요 (16b: 쿠션에서 쉬기, 16c: 쿠션 메뉴·쓰러짐·지역 전환).

#include "game/save/save_sources.hpp"

#include "wind/save/save_slots.hpp"

#include <filesystem>

namespace wind::client {

class SaveKeeper {
public:
    // folder: 저장 폴더, slot: 1~3, sources: 저장할 게임 상태(앱이 가지고 있어요)
    SaveKeeper(std::filesystem::path folder, int slot, game::SaveSources sources);

    // 실제 시간이 흘렀어요 (플레이 시간, 화면 한 장마다)
    void add_play_time(double seconds);
    // 불러온 저장의 플레이 시간에서 이어 세요
    void set_play_time(double seconds) { play_time_ = seconds; }

    // 지금 상태를 저장해요. hero: 지금 필드의 주인공 자리·HP·MP
    bool save(save::SaveKind kind, const game::HeroPlace& hero);

    [[nodiscard]] save::SaveSlots& slots() { return slots_; }
    [[nodiscard]] int slot() const { return slot_; }
    // 저장할 게임 상태 (불러올 때 apply_save가 같은 곳에 되살려요)
    [[nodiscard]] game::SaveSources& sources() { return sources_; }
    // 성공한 저장 횟수 (테스트·로그용)
    [[nodiscard]] unsigned saves_done() const { return saves_done_; }

private:
    save::SaveSlots slots_;
    int slot_;
    game::SaveSources sources_;
    double play_time_ = 0.0;   // 플레이 시간 (초)
    unsigned saves_done_ = 0;
};

}  // namespace wind::client
