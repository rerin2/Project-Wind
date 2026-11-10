// 이 파일은 무엇을 하나요?
// 맵 옮기기 순서(가리기 -> 새 맵 열기 -> 다시 보여 주기)를 진행하는 곳이에요.

#include "game/world/map_travel.hpp"

#include "wind/log.hpp"

#include <utility>

namespace wind::game {
namespace {

// 새 맵을 못 열었을 때 오류 글자를 화면에 보여 주는 시간 (초)
constexpr float kNoticeSeconds = 4.0f;

}  // namespace

MapTravel::MapTravel(float fade_seconds) : fade_(fade_seconds) {}

bool MapTravel::begin(world::MapExit exit) {
    if (fade_.busy()) {
        return false;
    }
    log::info("Exit to '{}' (spawn '{}')", exit.to_map, exit.to_spawn);
    fade_.fade_out(exit.transition);
    pending_ = std::move(exit);
    return true;
}

void MapTravel::update(float dt_seconds, const EnterMap& enter) {
    fade_.update(dt_seconds);
    if (notice_seconds_ > 0.0f) {
        notice_seconds_ -= dt_seconds;
        if (notice_seconds_ <= 0.0f) {
            notice_.clear();
        }
    }
    // 다 가려진 순간에만 새 맵을 열어요 (가려진 동안 바꿔야 갑자기 바뀌는 게 안 보여요).
    if (fade_.phase() != engine::ScreenFade::Phase::Covered || !pending_) {
        return;
    }
    const world::MapExit exit = std::move(*pending_);
    pending_.reset();
    if (enter(exit)) {
        notice_.clear();
        notice_seconds_ = 0.0f;
    } else {
        // 못 열었어요: 지금 맵에 그대로 있고, 왜 못 갔는지 기록하고 화면에도 잠깐 보여 줘요.
        log::error("Cannot enter '{}' (spawn '{}'); staying on the current map", exit.to_map,
                   exit.to_spawn);
        notice_ = "Cannot enter " + exit.to_map + " (spawn " + exit.to_spawn + ")";
        notice_seconds_ = kNoticeSeconds;
    }
    fade_.fade_in();
}

}  // namespace wind::game
