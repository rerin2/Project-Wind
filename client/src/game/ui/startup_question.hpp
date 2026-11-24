#pragma once

// 이 파일은 무엇을 하나요?
// 게임을 켤 때 저장 파일에 문제가 있으면 묻는 작은 질문의 규칙이에요. (명세 12.2, 16b, 개발자와 정함)
// - 손상(백업 있음): Enter "백업 불러오기", Esc "새로 시작"
// - 손상(쓸 만한 백업 없음): Enter·Esc 모두 "새로 시작"
// - 미래 버전: Enter·Esc 모두 "게임 끄기" (파일은 그대로)
// 키를 받아 답을 정하기만 해요. 그리기와 화면 반복은 앱(startup_prompt)이 해요. SDL을 몰라요.

#include <cstdint>
#include <optional>

namespace wind::game {

class StartupQuestion {
public:
    enum class Kind : std::uint8_t { Corrupt, TooNew };
    enum class Answer : std::uint8_t { UseBackup, StartFresh, Quit };

    StartupQuestion(Kind kind, bool backup_offered) : kind_(kind), backup_offered_(backup_offered) {}

    // 이번 장에 Enter(confirm)·Esc(back)를 눌렀을 때의 답. 아무것도 안 눌렀으면 nullopt
    [[nodiscard]] std::optional<Answer> answer(bool confirm, bool back) const;

    [[nodiscard]] Kind kind() const { return kind_; }
    // "Enter 백업 불러오기" 줄을 보일지
    [[nodiscard]] bool offers_backup() const { return kind_ == Kind::Corrupt && backup_offered_; }

private:
    Kind kind_;
    bool backup_offered_;
};

}  // namespace wind::game
