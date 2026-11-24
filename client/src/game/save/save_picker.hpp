#pragma once

// 이 파일은 무엇을 하나요?
// 게임을 켤 때 슬롯의 본 저장·자동 저장을 보고 "무엇으로 시작할지" 정해요. (명세 12.2·12.3, 16b)
// - 미래 버전 파일이 하나라도 있으면 TooNew (알림 뒤 게임 끄기, 파일은 그대로)
// - 손상 파일(모양이 틀림, 또는 콘텐츠와 안 맞음)이 하나라도 있으면 NeedsChoice (확인 창)
//     Enter "백업 불러오기" → use_backups, Esc "새로 시작" → set_aside_corrupt. 그다음 다시 examine
// - 아니면 올바른 파일 중 saved_at이 더 늦은 것을 Load (같으면 본 저장), 하나도 없으면 NewGame
// 화면·SDL을 몰라서 테스트할 수 있어요. 확인 창 그리기와 키는 앱(StartupPrompt)이 해요.

#include "wind/save/slot_read.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace wind::save {
class SaveSlots;
}

namespace wind::game {

class SavePicker {
public:
    // 저장 내용이 콘텐츠와 맞는지 보는 함수 (check_save_content). 틀리면 problems에 적고 false
    using Validate = std::function<bool(const save::SaveData&, std::vector<std::string>& problems)>;

    enum class Start : std::uint8_t { NewGame, Load, NeedsChoice, TooNew };

    struct Outcome {
        Start start = Start::NewGame;
        std::optional<save::SaveData> data;   // Load일 때 불러올 내용
        save::SaveKind from = save::SaveKind::Main;  // Load일 때 어느 파일인지 (로그용)
        bool backup_offered = false;          // NeedsChoice일 때 "백업 불러오기"를 보일지
        std::vector<std::string> problems;    // 왜 손상·미래 버전인지 (로그용)
    };

    SavePicker(save::SaveSlots& slots, int slot, Validate validate);

    // 두 파일을 읽어 보고 정해요 (파일은 바꾸지 않아요)
    [[nodiscard]] Outcome examine() const;
    // 손상 파일마다: 쓸 만한 백업이 있으면 손상본을 .corrupt로 치우고 백업으로 되살려요, 없으면 치우기만.
    // 실패하면 false와 이유
    bool use_backups(std::string& error);
    // 손상 파일을 모두 .corrupt로 치워요 (새로 시작. 올바른 다른 파일은 그대로 남아 다음 examine이 불러요)
    bool set_aside_corrupt(std::string& error);

private:
    // 파일 하나를 읽고 콘텐츠까지 확인한 결과 (콘텐츠가 안 맞으면 Corrupt로 바꿔요)
    struct Checked {
        save::SaveStatus status = save::SaveStatus::Missing;
        std::optional<save::SaveData> data;
        std::vector<std::string> problems;
        bool backup_usable = false;  // 손상일 때만: 백업이 모양·콘텐츠 모두 맞나
    };
    [[nodiscard]] Checked check(save::SaveKind kind) const;
    // 이 내용이 쓸 만한가 (Ok + 콘텐츠 확인)
    [[nodiscard]] bool usable(const save::SlotRead& read, std::vector<std::string>& problems) const;

    save::SaveSlots& slots_;
    int slot_;
    Validate validate_;
};

}  // namespace wind::game
