// 이 파일은 무엇을 하나요?
// 능력치 곡선 파일과 레벨 비용 파일을 한 칸씩 확인하며 읽어요.

#include "wind/content/progression_loader.hpp"

#include "wind/content/content_path.hpp"
#include "wind/content/json_file.hpp"
#include "wind/content/json_reader.hpp"
#include "wind/progression/stats.hpp"

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace wind::content {
namespace {

constexpr std::size_t kMaxCurvePoints = 20;  // 곡선 하나의 점 상한
constexpr double kMaxCurveValue = 99999.0;   // 곡선 값 상한 (HP 상한과 같아요)
constexpr double kMinCostBase = 1.0;         // a 최소: 비용이 늘 1 이상이 되게
constexpr double kMaxCostBase = 1'000'000.0; // a 최대
constexpr double kMaxCostLinear = 100'000.0; // b 최대
constexpr double kMaxCostSquare = 10'000.0;  // c 최대 (레벨 250에서도 int64에 넉넉히 들어가요)

// 곡선 하나 읽기: [[능력치, 값], ...]. 틀리면 공책에 적고 nullopt.
std::optional<progression::StatCurve> read_curve(JsonReader& reader, std::string_view key) {
    const auto* list = reader.array(key, kMaxCurvePoints);  // 없거나 너무 많으면 공책에 적혀요
    if (list == nullptr) return std::nullopt;
    if (list->empty()) {
        reader.error(reader.field_path(key), "needs at least one point");
        return std::nullopt;
    }
    std::vector<progression::CurvePoint> points;
    bool ok = true;
    for (std::size_t i = 0; i < list->size(); ++i) {
        const auto field = reader.index_path(key, i);
        const auto& item = (*list)[i];
        // [정수 능력치, 숫자 값] 두 개짜리 목록이어야 해요
        if (!item.is_array() || item.size() != 2 || !item[0].is_number_integer() || !item[1].is_number()) {
            reader.error(field, "expected [stat, value]");
            ok = false;
            continue;
        }
        const auto stat = item[0].get<std::int64_t>();
        const auto value = item[1].get<double>();
        if (stat < progression::kStatMin || stat > progression::kStatMax) {  // 능력치는 1~60
            reader.error(field, "stat must be 1..60");
            ok = false;
            continue;
        }
        if (!(value >= 0.0 && value <= kMaxCurveValue)) {  // NaN도 여기서 걸러요
            reader.error(field, "value must be 0..99999");
            ok = false;
            continue;
        }
        if (!points.empty() && stat <= points.back().stat) {  // 오름차순 (같은 능력치 두 번도 안 돼요)
            reader.error(field, "stats must be in increasing order");
            ok = false;
            continue;
        }
        points.push_back({static_cast<int>(stat), value});
    }
    if (!ok) return std::nullopt;
    return progression::StatCurve(std::move(points));
}

// JSON 파일 하나를 읽어 parse 함수에 넘겨요 (경로는 콘텐츠 폴더 안으로 제한)
template <typename T, typename Parse>
LoadResult<T> load_file(const std::filesystem::path& content_root, std::string_view path, Parse parse) {
    ErrorList errors;
    const std::string name(path);
    const auto file = resolve_content_path(content_root, path);
    if (!file) {
        errors.add(name, "", "path must stay inside the content folder");
        return {std::nullopt, errors.take()};
    }
    const auto document = read_json_file(*file, name, errors);
    if (!document) return {std::nullopt, errors.take()};
    return parse(*document, name);
}

}  // namespace

LoadResult<progression::StatCurves> parse_stat_curves(const nlohmann::json& document, const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    if (!reader.valid() || !reader.schema_version(kStatCurvesSchemaVersion)) {
        if (errors.empty()) errors.add(file, "", "expected a JSON object");
        return {std::nullopt, errors.take()};
    }
    // 곡선 일곱 개를 모두 읽어요 (하나가 틀려도 나머지 오류도 함께 보이게 끝까지 읽어요)
    auto hp = read_curve(reader, "max_hp");
    auto mp = read_curve(reader, "max_mp");
    auto stamina = read_curve(reader, "max_stamina");
    auto weight = read_curve(reader, "carry_weight");
    auto strength = read_curve(reader, "strength_bonus");
    auto intelligence = read_curve(reader, "intelligence_bonus");
    auto slots = read_curve(reader, "spell_slots");
    if (!hp || !mp || !stamina || !weight || !strength || !intelligence || !slots || !errors.empty()) {
        if (errors.empty()) errors.add(file, "", "stat curves are incomplete");
        return {std::nullopt, errors.take()};
    }
    progression::StatCurves curves;
    curves.max_hp = std::move(*hp);
    curves.max_mp = std::move(*mp);
    curves.max_stamina = std::move(*stamina);
    curves.carry_weight = std::move(*weight);
    curves.strength_bonus = std::move(*strength);
    curves.intelligence_bonus = std::move(*intelligence);
    curves.spell_slots = std::move(*slots);
    return {std::move(curves), {}};
}

LoadResult<progression::LevelCostRules> parse_level_rules(const nlohmann::json& document,
                                                          const std::string& file) {
    ErrorList errors;
    JsonReader reader(document, file, "", errors);
    if (!reader.valid() || !reader.schema_version(kLevelRulesSchemaVersion)) {
        if (errors.empty()) errors.add(file, "", "expected a JSON object");
        return {std::nullopt, errors.take()};
    }
    const auto a = reader.number("a", kMinCostBase, kMaxCostBase);  // 기본 비용
    const auto b = reader.number("b", 0.0, kMaxCostLinear);         // 레벨마다 더하는 비용
    const auto c = reader.number("c", 0.0, kMaxCostSquare);         // 제곱으로 커지는 몫
    if (!a || !b || !c || !errors.empty()) {
        if (errors.empty()) errors.add(file, "", "level rules are incomplete");
        return {std::nullopt, errors.take()};
    }
    return {progression::LevelCostRules{*a, *b, *c}, {}};
}

LoadResult<progression::ProgressionRules> load_progression_rules(const std::filesystem::path& content_root) {
    auto curves = load_file<progression::StatCurves>(content_root, kStatCurvesPath, parse_stat_curves);
    auto levels = load_file<progression::LevelCostRules>(content_root, kLevelRulesPath, parse_level_rules);
    if (!curves.ok() || !levels.ok()) {
        // 두 파일의 오류를 모두 모아 돌려줘요
        std::vector<ContentError> errors = std::move(curves.errors);
        errors.insert(errors.end(), levels.errors.begin(), levels.errors.end());
        return {std::nullopt, std::move(errors)};
    }
    return {progression::ProgressionRules{std::move(*curves.value), *levels.value}, {}};
}

}  // namespace wind::content
