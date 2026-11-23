#pragma once

// 이 파일은 무엇을 하나요?
// 마술 선반이에요. (15b) 배운 마술의 데이터(content/spells/<id>.json)를 미리 읽어 둬요.
// Q로 마술을 바꿀 때 파일을 다시 읽지 않고 여기서 꺼내 써요 (무기 선반 WeaponRack과 같은 생각).
// 파일 이름은 마술 id로 정해요: "moon_blade" → spells/moon_blade.json. 파일 속 id가 다르면 거절해요.
// 게임 규칙 쪽 물건이라 SDL을 몰라요.

#include "wind/combat/spell.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace wind::game {

class SpellShelf {
public:
    // spells/<id>.json을 읽어 올려요. 이미 있으면 그대로 true, 못 읽으면 이유를 기록하고 false
    bool load(const std::filesystem::path& content_root, std::string_view id);
    // 마술을 직접 올려요 (테스트용)
    void add(combat::Spell spell);
    // 이 id의 마술 (없으면 nullptr)
    [[nodiscard]] const combat::Spell* find(std::string_view id) const;
    [[nodiscard]] std::size_t size() const { return spells_.size(); }

private:
    std::map<std::string, combat::Spell, std::less<>> spells_;  // 마술 id → 마술
};

}  // namespace wind::game
