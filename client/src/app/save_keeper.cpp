// 이 파일은 무엇을 하나요?
// 저장 담당의 실제 동작: 상태 모으기 → 때·시간·버전 붙이기 → 안전하게 쓰기 → 결과 기록.

#include "app/save_keeper.hpp"

#include "game/save/save_capture.hpp"

#include "wind/log.hpp"
#include "wind/version.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>

namespace wind::client {

SaveKeeper::SaveKeeper(std::filesystem::path folder, int slot, game::SaveSources sources)
    : slots_(std::move(folder)), slot_(slot), sources_(sources) {}

void SaveKeeper::add_play_time(double seconds) {
    play_time_ += std::max(seconds, 0.0);
}

bool SaveKeeper::save(save::SaveKind kind, const game::HeroPlace& hero) {
    const auto started = std::chrono::steady_clock::now();  // 저장 시간 재기 (명세 16.1: 0.1초 이하)
    auto data = game::capture_save(sources_, hero);
    data.game_version = std::string(kVersion);
    data.saved_at = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();  // 유닉스 초 (UTC)
    data.play_time_seconds = play_time_;
    std::string error;
    const char* kind_name = kind == save::SaveKind::Main ? "main" : "auto";
    if (!slots_.write(slot_, kind, data, error)) {
        log::error("Save failed (slot {} {}): {}", slot_, kind_name, error);
        return false;
    }
    ++saves_done_;
    const auto ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    log::info("Saved slot {} {} at {} ({:.0f}, {:.0f}) in {:.1f} ms", slot_, kind_name, hero.map, hero.feet.x,
              hero.feet.y, ms);
    return true;
}

}  // namespace wind::client
