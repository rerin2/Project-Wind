// 이 파일은 무엇을 하나요?
// 그림 빌려주는 사서(ResourceManager)가 실제로 일하는 방법을 적은 곳이에요.
// 1) 파일 위치가 안전한지 확인  2) 책상 위(캐시)에 있으면 바로 주기
// 3) 없으면 파일을 읽기  4) 실패하면 체크무늬 그림 주기

#include "engine/resources/resource_manager.hpp"

#include "wind/content/content_path.hpp"
#include "wind/log.hpp"

#include <utility>

namespace wind::engine {
namespace {

// 대신 줄 체크무늬 그림의 크기(픽셀). 타일 한 칸 크기와 같아요.
constexpr std::int32_t kFallbackTextureSize = 32;

}  // namespace

ResourceManager::ResourceManager(Renderer& renderer, std::filesystem::path content_root)
    : renderer_(renderer), root_(std::move(content_root)) {}

// 퇴근할 때: 빌려준 그림을 모두 정리하고, 체크무늬 그림도 버려요.
ResourceManager::~ResourceManager() {
    release_all();
    if (fallback_.valid()) {
        renderer_.destroy_texture(fallback_);
    }
}

TextureHandle ResourceManager::texture(std::string_view relative_path) {
    // 같은 파일을 "a\\b.bmp", "./a/b.bmp"처럼 다르게 적어도 한 번만 읽도록, 먼저 모양을 통일해요.
    // 이때 콘텐츠 폴더 밖을 가리키는 위험한 위치는 거절돼요.
    const auto normalized = content::normalize_content_path(relative_path);
    if (!normalized) {
        const std::string raw(relative_path);
        if (failed_.insert(raw).second) {  // 처음 실패한 위치일 때만 기록해요
            ++stats_.load_failures;
            log::error("Rejected texture path outside the content folder: '{}'", raw);
        }
        return fallback();
    }

    // 이미 책상 위에 꺼내 둔 그림이면 바로 줘요. (캐시 적중)
    if (const auto it = textures_.find(*normalized); it != textures_.end()) {
        ++stats_.cache_hits;
        return it->second;
    }
    if (failed_.contains(*normalized)) {
        return fallback();  // 전에 실패한 파일은 다시 읽지도, 오류를 또 적지도 않아요
    }

    // 기록에 '\'와 '/'가 섞이지 않도록 컴퓨터(OS)가 쓰는 구분 기호로 통일해요.
    auto path = root_ / std::filesystem::path(*normalized);
    path.make_preferred();
    std::string error;
    const TextureHandle handle = renderer_.load_bmp_texture(path, error);
    if (!handle.valid()) {
        // 읽기 실패: 실패 목록에 적고, 왜 실패했는지 기록한 뒤 체크무늬를 줘요.
        failed_.insert(*normalized);
        ++stats_.load_failures;
        log::error("Failed to load texture '{}' ({}): {}", *normalized, path.string(), error);
        return fallback();
    }

    // 성공: 책상 위(캐시)에 올려 두고 번호표를 줘요.
    textures_.emplace(*normalized, handle);
    stats_.textures_loaded = textures_.size();
    log::debug("Loaded texture '{}'", *normalized);
    return handle;
}

void ResourceManager::release(std::string_view relative_path) {
    const auto normalized = content::normalize_content_path(relative_path);
    if (!normalized) {
        return;
    }
    if (const auto it = textures_.find(*normalized); it != textures_.end()) {
        renderer_.destroy_texture(it->second);
        textures_.erase(it);
    }
    failed_.erase(*normalized);  // 파일을 고친 뒤 다시 읽어 볼 수 있게 실패 기록도 지워요
    stats_.textures_loaded = textures_.size();
}

// 책상 위의 그림을 모두 치우고 실패 기록도 지워요.
void ResourceManager::release_all() {
    for (const auto& [path, handle] : textures_) {
        renderer_.destroy_texture(handle);
    }
    textures_.clear();
    failed_.clear();
    stats_.textures_loaded = 0;
}

// 체크무늬 그림은 처음 필요할 때 딱 한 번 만들고 계속 같은 걸 써요.
TextureHandle ResourceManager::fallback() {
    if (!fallback_.valid()) {
        fallback_ = renderer_.create_checker_texture(kFallbackTextureSize);
    }
    return fallback_;
}

}  // namespace wind::engine
