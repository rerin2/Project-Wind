// 이 파일은 무엇을 하나요?
// 시작 질문의 답 정하기 (startup_question.hpp 설명 참고).

#include "game/ui/startup_question.hpp"

namespace wind::game {

std::optional<StartupQuestion::Answer> StartupQuestion::answer(bool confirm, bool back) const {
    if (!confirm && !back) return std::nullopt;
    if (kind_ == Kind::TooNew) return Answer::Quit;           // 어느 키든 끄기
    if (offers_backup() && confirm) return Answer::UseBackup;  // Enter = 백업
    return Answer::StartFresh;                                 // Esc, 또는 백업이 없으면 어느 키든
}

}  // namespace wind::game
