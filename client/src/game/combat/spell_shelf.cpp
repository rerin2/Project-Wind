// 이 파일은 무엇을 하나요?
// 마술 선반: 마술 파일을 id로 찾아 읽어 둬요.

#include "game/combat/spell_shelf.hpp"

#include "wind/content/error_list.hpp"
#include "wind/content/spell_loader.hpp"
#include "wind/log.hpp"

#include <utility>

namespace wind::game {

bool SpellShelf::load(const std::filesystem::path& content_root, std::string_view id) {
    if (find(id) != nullptr) return true;  // 이미 읽었어요
    const std::string path = "spells/" + std::string(id) + ".json";
    auto loaded = content::load_spell_file(content_root, path);  // 콘텐츠 폴더 밖은 못 열어요
    if (!loaded.ok()) {
        content::log_content_errors("spell " + path, loaded.errors);
        return false;
    }
    if (loaded.value->id != id) {  // 파일 이름과 속 id가 다르면 저장 파일과 어긋나요
        log::error("Spell file {} has id {}, expected {}", path, loaded.value->id, id);
        return false;
    }
    add(std::move(*loaded.value));
    return true;
}

void SpellShelf::add(combat::Spell spell) {
    std::string id = spell.id;
    spells_[std::move(id)] = std::move(spell);
}

const combat::Spell* SpellShelf::find(std::string_view id) const {
    const auto it = spells_.find(id);
    return it != spells_.end() ? &it->second : nullptr;
}

}  // namespace wind::game
