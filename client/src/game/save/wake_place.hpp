#pragma once

// 이 파일은 무엇을 하나요?
// 쓰러진 순간에 "깨어날 자리"를 미리 구해요. (명세 12.3, 16c, 2026-11-24 개발자와 정함)
// 쓰러지면 룬을 잃은 그 박자에 저장하는데, 저장 내용은 "깨어난 뒤 상태"예요:
// 마지막 쿠션 자리(쉰 적이 없으면 지금 맵의 기본 자리), HP·MP 가득.
// 부활할 맵이 지금 맵과 다를 수 있어서, 그 맵 파일만 잠깐 읽어 시작 칸을 찾아요(필드는 만들지 않아요).
// 그래서 쓰러짐 화면 중에 게임을 강제로 꺼도 잃은 룬이 되살아나지 않아요.

#include "game/save/save_sources.hpp"

#include <filesystem>
#include <optional>
#include <string_view>

namespace wind::game {

class HeroProgress;

// content_root: 맵 파일을 읽을 곳, current_map: 쓰러진 맵, full_hp·full_mp: 가득 찬 HP·MP.
// 맵을 못 읽거나 시작 칸이 없으면 없음 (이유는 로그에)
[[nodiscard]] std::optional<HeroPlace> wake_place(const std::filesystem::path& content_root,
                                                  const HeroProgress& progress, std::string_view current_map,
                                                  int full_hp, float full_mp);

}  // namespace wind::game
