// 이 파일은 무엇을 하나요?
// 시작할 때 저장 고르기·질문·되살리기의 실제 순서예요 (game_start.hpp 설명 참고).

#include "app/game_start.hpp"

#include "app/client_options.hpp"
#include "app/item_setup.hpp"
#include "app/save_keeper.hpp"
#include "app/startup_prompt.hpp"
#include "engine/platform/sdl_platform.hpp"
#include "game/save/save_apply.hpp"
#include "game/save/save_content_check.hpp"
#include "game/save/save_picker.hpp"

#include "wind/log.hpp"

#include <limits>

namespace wind::client {
namespace {

// 질문은 많아야 한 번(손상) 나오고 다시 확인하면 끝나요. 혹시 모를 무한 반복을 막는 상한
constexpr int kMaxRounds = 3;

// 불러온 저장으로 시작점 만들기 (스태미나는 저장하지 않아 가득: 큰 값을 넣으면 최대로 잘려요)
StartPoint loaded_start(const save::SaveData& data) {
    game::HeroVitals vitals;
    vitals.hp = data.hp;
    vitals.mana = data.mp;
    vitals.stamina = std::numeric_limits<float>::max();
    return {data.position.map, engine::Vec2{data.position.x, data.position.y}, vitals};
}

// 손상 질문에 답한 대로 파일을 정리해요. 실패하면 false (안전하게 계속할 수 없어 게임을 꺼요)
bool resolve_corrupt(game::SavePicker& picker, game::StartupQuestion::Answer answer) {
    std::string error;
    const bool ok = answer == game::StartupQuestion::Answer::UseBackup ? picker.use_backups(error)
                                                                        : picker.set_aside_corrupt(error);
    if (!ok) log::critical("Could not tidy the save files: {}", error);
    return ok;
}

}  // namespace

std::filesystem::path save_folder(const ClientOptions& options) {
    if (!options.save_dir.empty()) return options.save_dir;
    const auto user_dir = engine::SdlPlatform::user_data_dir();
    if (user_dir.empty()) {
        log::warn("No user data folder; saving into ./saves");
        return "saves";
    }
    return user_dir / "saves";
}

std::optional<StartPoint> choose_start(const ClientOptions& options, SaveKeeper& saver, ItemSetup& items,
                                       engine::SdlPlatform& platform, engine::UiPainter& painter,
                                       const game::InputBindings& bindings, std::uint64_t& frame) {
    const StartPoint fresh{options.map, std::nullopt, std::nullopt};
    if (options.new_game) {
        log::info("--new-game: starting a new game in slot {} (the old save becomes the backup on the first save)",
                  saver.slot());
        return fresh;
    }
    // 콘텐츠 확인: 아이템·마술·맵·쿠션 (배운 마술은 여기서 선반에 올라가요)
    const game::SaveContent content{options.content_dir, items.catalog, items.weapons, items.spells};
    game::SavePicker picker(saver.slots(), saver.slot(), [&content](const save::SaveData& data, auto& problems) {
        return game::check_save_content(data, content, problems);
    });
    for (int round = 0; round < kMaxRounds; ++round) {
        auto outcome = picker.examine();
        for (const auto& problem : outcome.problems) log::warn("Save slot {}: {}", saver.slot(), problem);
        switch (outcome.start) {
        case game::SavePicker::Start::NewGame:
            log::info("Slot {} is empty: new game", saver.slot());
            return fresh;
        case game::SavePicker::Start::TooNew:
            (void)ask_startup_question({game::StartupQuestion::Kind::TooNew, false}, platform, painter, bindings,
                                       options, frame);
            log::critical("Slot {} was saved by a newer game version; quitting without touching it", saver.slot());
            return std::nullopt;
        case game::SavePicker::Start::NeedsChoice: {
            const auto answer = ask_startup_question({game::StartupQuestion::Kind::Corrupt, outcome.backup_offered},
                                                     platform, painter, bindings, options, frame);
            if (answer == game::StartupQuestion::Answer::Quit || !resolve_corrupt(picker, answer)) return std::nullopt;
            log::info("Corrupt save {}", answer == game::StartupQuestion::Answer::UseBackup ? "restored from backup"
                                                                                           : "set aside");
            continue;  // 정리한 뒤 다시 확인해요
        }
        case game::SavePicker::Start::Load: {
            if (has_test_start_options(options)) log::warn("Test start options are ignored because a save was loaded");
            if (!game::apply_save(*outcome.data, saver.sources(), items.catalog)) return std::nullopt;
            saver.set_play_time(outcome.data->play_time_seconds);
            log::info("Loaded slot {} {} save: {} ({:.0f}, {:.0f})", saver.slot(),
                      outcome.from == save::SaveKind::Main ? "main" : "auto", outcome.data->position.map,
                      outcome.data->position.x, outcome.data->position.y);
            return loaded_start(*outcome.data);
        }
        }
    }
    log::critical("Save files kept changing while starting; quitting");
    return std::nullopt;
}

}  // namespace wind::client
