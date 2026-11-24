// 이 파일은 무엇을 하나요?
// 시작할 때 저장 고르기의 실제 순서예요 (save_picker.hpp 설명 참고).

#include "game/save/save_picker.hpp"

#include "wind/content/error_list.hpp"
#include "wind/save/save_slots.hpp"

#include <array>
#include <utility>

namespace wind::game {
namespace {

using save::SaveKind;
using save::SaveStatus;

constexpr std::array<SaveKind, 2> kKinds{SaveKind::Main, SaveKind::Auto};

const char* kind_name(SaveKind kind) {
    return kind == SaveKind::Main ? "main" : "auto";
}

}  // namespace

SavePicker::SavePicker(save::SaveSlots& slots, int slot, Validate validate)
    : slots_(slots), slot_(slot), validate_(std::move(validate)) {}

bool SavePicker::usable(const save::SlotRead& read, std::vector<std::string>& problems) const {
    return read.status == SaveStatus::Ok && read.data && validate_(*read.data, problems);
}

SavePicker::Checked SavePicker::check(SaveKind kind) const {
    Checked checked;
    auto read = slots_.read(slot_, kind);
    checked.status = read.status;
    for (const auto& e : read.errors) checked.problems.push_back(content::to_string(e));
    // 모양은 맞는데 콘텐츠와 안 맞으면 손상으로 봐요 (개발자와 정함)
    if (read.status == SaveStatus::Ok && !validate_(*read.data, checked.problems)) checked.status = SaveStatus::Corrupt;
    if (checked.status == SaveStatus::Ok) {
        checked.data = std::move(read.data);
    } else if (checked.status == SaveStatus::Corrupt) {
        std::vector<std::string> ignored;  // 백업의 문제는 확인 창에 보이지 않아요
        checked.backup_usable = usable(slots_.read_backup(slot_, kind), ignored);
    }
    return checked;
}

SavePicker::Outcome SavePicker::examine() const {
    Outcome outcome;
    std::array<Checked, kKinds.size()> files{check(kKinds[0]), check(kKinds[1])};
    bool too_new = false;
    bool corrupt = false;
    for (std::size_t i = 0; i < files.size(); ++i) {
        too_new = too_new || files[i].status == SaveStatus::TooNew;
        corrupt = corrupt || files[i].status == SaveStatus::Corrupt;
        if (files[i].status == SaveStatus::Corrupt) outcome.backup_offered = outcome.backup_offered || files[i].backup_usable;
        for (auto& p : files[i].problems) outcome.problems.push_back(std::string(kind_name(kKinds[i])) + ": " + p);
    }
    // 1) 미래 버전이 먼저: 손대면 안 되는 파일이 있어요
    if (too_new) {
        outcome.start = Start::TooNew;
        return outcome;
    }
    // 2) 손상 파일이 있으면 사람이 정해요
    if (corrupt) {
        outcome.start = Start::NeedsChoice;
        return outcome;
    }
    // 3) 올바른 파일 중 더 새것 (같으면 본 저장이 앞이라 본 저장)
    std::optional<std::size_t> best;
    for (std::size_t i = 0; i < files.size(); ++i) {
        if (files[i].data && (!best || files[i].data->saved_at > files[*best].data->saved_at)) best = i;
    }
    if (best) {
        outcome.start = Start::Load;
        outcome.from = kKinds[*best];
        outcome.data = std::move(files[*best].data);
    }
    return outcome;
}

bool SavePicker::use_backups(std::string& error) {
    for (const auto kind : kKinds) {
        const auto checked = check(kind);
        if (checked.status != SaveStatus::Corrupt) continue;
        // 손상본을 먼저 치우고(콘텐츠만 안 맞는 파일도 지우지 않게), 쓸 만한 백업이면 되살려요
        if (!slots_.set_aside(slot_, kind, error)) return false;
        if (checked.backup_usable && !slots_.restore_backup(slot_, kind, error)) return false;
    }
    return true;
}

bool SavePicker::set_aside_corrupt(std::string& error) {
    for (const auto kind : kKinds) {
        if (check(kind).status == SaveStatus::Corrupt && !slots_.set_aside(slot_, kind, error)) return false;
    }
    return true;
}

}  // namespace wind::game
