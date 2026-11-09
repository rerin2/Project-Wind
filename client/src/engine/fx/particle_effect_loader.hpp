#pragma once

// 이 파일은 무엇을 하나요?
// 파티클 효과 요리법(content/fx/*.json)을 읽고 숙제 검사를 하는 도우미예요. (명세 14.3, 14.4)
// 숫자가 범위를 벗어나거나, 모르는 이름이 있거나, 칸이 빠지면
// "어느 파일의 어느 칸이 왜 틀렸는지"를 오류 목록에 적고 효과를 쓰지 않아요.
// 형식은 docs/ENGINE.md "Particle effect" 절에 적어 뒀어요.

#include "engine/fx/particle_effect.hpp"

#include "wind/content/load_result.hpp"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace wind::engine {

inline constexpr std::int64_t kParticleEffectSchemaVersion = 1;

// 이미 읽어 둔 JSON을 검사해요. file은 오류 메시지에 보여 줄 파일 이름이에요.
[[nodiscard]] content::LoadResult<ParticleEffect> parse_particle_effect(
    const nlohmann::json& document, const std::string& file);

// 콘텐츠 폴더 안의 효과 파일을 읽고 검사해요.
[[nodiscard]] content::LoadResult<ParticleEffect> load_particle_effect_file(
    const std::filesystem::path& content_root, std::string_view path);

}  // namespace wind::engine
