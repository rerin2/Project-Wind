// 이 파일은 무엇을 하나요?
// 몬스터 설명 파일을 한 칸씩 확인하며 읽는 곳이에요. (11a)

#include "wind/content/monster_loader.hpp"

#include "wind/content/body_readers.hpp"
#include "wind/content/combat_readers.hpp"
#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"

#include <optional>
#include <utility>

namespace wind::content {
namespace {

constexpr std::size_t kMaxIdLength = 64;    // id 글자 수 상한 (바이트)
constexpr std::size_t kMaxNameLength = 64;  // 이름 상한 (바이트, 한글 한 글자 = 3바이트)
constexpr std::int64_t kMaxHp = 99999;      // HP 상한 (피해 상한과 같아요)
constexpr double kMaxPower = 9999.0;        // 공격력·방어력 상한
constexpr double kMinSpeed = 1.0;           // 빠르기 최소 (0이면 영원히 못 움직여요, 픽셀/초)
constexpr double kMaxSpeed = 400.0;         // 빠르기 최대 (구르기 속도쯤)
constexpr double kMaxRange = 2000.0;        // 감지·공격·추적 거리 최대 (픽셀)
constexpr double kMaxWanderRadius = 512.0;  // 배회 반지름 최대 (픽셀)
constexpr double kMaxPause = 30.0;          // 배회 쉬는 시간 최대 (초)
constexpr double kMinBody = 1.0;            // 발 상자 최소 (픽셀)
constexpr double kMaxBody = 64.0;           // 발 상자 최대 (두 칸)

// "wander": { "radius": 40, "speed": 18, "pause": [1.0, 2.5] }
std::optional<combat::WanderData> read_wander(JsonReader& parent) {
    const auto* object = parent.object("wander");
    if (object == nullptr) return std::nullopt;
    JsonReader reader(*object, parent.file(), parent.field_path("wander"), parent.errors());
    const auto radius = reader.number("radius", 0.0, kMaxWanderRadius);  // 0이면 제자리에서만 쉬어요
    const auto speed = reader.number("speed", kMinSpeed, kMaxSpeed);     // 배회 걸음
    const auto pause = read_number_pair(reader, "pause", 0.0, kMaxPause);  // [최소, 최대] 쉬는 시간
    if (!radius || !speed || !pause) return std::nullopt;
    if ((*pause)[0] > (*pause)[1]) {  // 최소가 최대보다 크면 틀린 것
        reader.error(reader.field_path("pause"), "expected [min, max] with min <= max");
        return std::nullopt;
    }
    return combat::WanderData{static_cast<float>(*radius), static_cast<float>(*speed),
                              static_cast<float>((*pause)[0]), static_cast<float>((*pause)[1])};
}

// 거리 순서 검사: 공격 거리 <= 감지 거리 <= 추적 한계 거리, 배회 반지름 <= 추적 한계 거리.
// 순서가 틀리면 "알아채기 전에 공격"이나 "배회하다 바로 포기" 같은 이상한 행동이 나와요.
void check_ranges(const combat::MonsterData& data, JsonReader& reader) {
    if (data.attack_range > data.detect_range) {
        reader.error("attack_range", "must not be larger than detect_range");
    }
    if (data.detect_range > data.leash_range) {
        reader.error("detect_range", "must not be larger than leash_range");
    }
    if (data.wander.radius > data.leash_range) {
        reader.error("wander.radius", "must not be larger than leash_range");
    }
}

}  // namespace

LoadResult<combat::MonsterData> parse_monster(const nlohmann::json& document, const std::string& file) {
    ErrorList errors;                               // 틀린 곳 공책
    JsonReader reader(document, file, "", errors);  // 맨 바깥 { } 읽기 도우미
    if (!reader.valid() || !reader.schema_version(kMonsterSchemaVersion)) {
        if (errors.empty()) errors.add(file, "", "expected a JSON object");
        return {std::nullopt, errors.take()};
    }
    auto id = reader.string("id", kMaxIdLength);                              // 이름표
    auto name = reader.string("name", kMaxNameLength);                        // 보여 줄 이름
    const auto max_hp = reader.integer("max_hp", 1, kMaxHp);                  // 가득 찬 HP
    const auto attack = reader.number("attack_power", 0.0, kMaxPower);        // 공격력
    const auto defense = reader.number("defense", 0.0, kMaxPower);            // 방어력
    const auto move_speed = reader.number("move_speed", kMinSpeed, kMaxSpeed);  // 쫓는 빠르기
    const auto detect = reader.number("detect_range", 1.0, kMaxRange);        // 알아채는 거리
    const auto attack_range = reader.number("attack_range", 1.0, kMaxRange);  // 공격 예고 거리
    const auto leash = reader.number("leash_range", 1.0, kMaxRange);          // 추적 한계 거리
    const auto wander = read_wander(reader);                                  // 배회
    const auto body = read_number_pair(reader, "body", kMinBody, kMaxBody);   // 발 상자 [가로, 세로]
    const auto hurtbox = read_hurtbox(reader);                                // 맞는 상자
    auto sprite = read_sprite_strip(reader);                                  // 그림 띠
    if (!id || !name || !max_hp || !attack || !defense || !move_speed || !detect || !attack_range ||
        !leash || !wander || !body || !hurtbox || !sprite) {
        if (errors.empty()) errors.add(file, "", "monster is incomplete");
        return {std::nullopt, errors.take()};
    }
    combat::MonsterData data;  // 칸마다 통과! 값을 담고 서로의 관계를 확인해요
    data.id = std::move(*id);
    data.name = std::move(*name);
    data.max_hp = static_cast<int>(*max_hp);
    data.attack_power = static_cast<float>(*attack);
    data.defense = static_cast<float>(*defense);
    data.move_speed = static_cast<float>(*move_speed);
    data.detect_range = static_cast<float>(*detect);
    data.attack_range = static_cast<float>(*attack_range);
    data.leash_range = static_cast<float>(*leash);
    data.wander = *wander;
    data.body_width = static_cast<float>((*body)[0]);
    data.body_height = static_cast<float>((*body)[1]);
    data.hurtbox = *hurtbox;
    data.sprite = std::move(*sprite);
    check_ranges(data, reader);  // 거리 순서
    if (!errors.empty()) {
        return {std::nullopt, errors.take()};
    }
    return {std::move(data), {}};
}

LoadResult<combat::MonsterData> load_monster_file(const std::filesystem::path& content_root,
                                                  std::string_view path) {
    ErrorList errors;
    const std::string name(path);
    const auto file = resolve_content_path(content_root, path);  // 콘텐츠 폴더 안인지 문지기 검사
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = read_json_file(*file, name, errors);  // 파일 읽어서 JSON으로
    if (!document) return {std::nullopt, errors.take()};
    return parse_monster(*document, name);  // 칸마다 검사
}

}  // namespace wind::content
