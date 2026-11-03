#pragma once

// 이 파일은 무엇을 하나요?
// 리소스 관리자 = 그림 파일을 빌려주는 "도서관 사서"예요. (명세 5.7)
// - 콘텐츠 폴더(게임 자료 폴더) 안의 파일만 빌려줘요. 밖의 파일은 안 돼요.
// - 같은 그림을 두 번 부탁하면, 다시 읽지 않고 책상 위에 꺼내 둔 걸 줘요. (이걸 캐시라고 해요)
// - 그림 파일이 없거나 망가져도 게임이 멈추지 않게, 대신 체크무늬 그림을 줘요.
//   오류는 파일마다 딱 한 번만 기록해서 기록이 같은 말로 꽉 차지 않게 해요.

#include "engine/render/renderer.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace wind::engine {

// 사서의 장부 (디버그 화면에 보여 줘요)
struct ResourceStats {
    std::size_t textures_loaded = 0;  // 지금 책상 위(캐시)에 있는 그림 수
    std::uint64_t cache_hits = 0;     // 이미 꺼내 둔 걸 다시 준 횟수
    std::uint64_t load_failures = 0;  // 읽기에 실패한 파일 수
};

class ResourceManager {
public:
    // 화가(Renderer)와 콘텐츠 폴더 위치를 알려 주며 사서를 만들어요.
    ResourceManager(Renderer& renderer, std::filesystem::path content_root);
    // 사서가 퇴근할 때 꺼내 둔 그림을 모두 정리해요. (RAII: 쓴 물건은 자동으로 제자리에)
    ~ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // 콘텐츠 폴더 안의 BMP 그림을 빌려줘요. 실패하면 체크무늬 그림을 줘요.
    [[nodiscard]] TextureHandle texture(std::string_view relative_path);

    // 그림을 반납해요. 다시 부탁하면 파일에서 새로 읽어요.
    void release(std::string_view relative_path);
    // 모든 그림을 반납해요.
    void release_all();

    [[nodiscard]] const ResourceStats& stats() const { return stats_; }
    [[nodiscard]] const std::filesystem::path& content_root() const { return root_; }

private:
    // 대신 줄 체크무늬 그림. 처음 필요할 때 한 번만 만들어요.
    TextureHandle fallback();

    Renderer& renderer_;
    std::filesystem::path root_;  // 콘텐츠 폴더 위치
    std::unordered_map<std::string, TextureHandle> textures_;  // 정리한 파일 위치 -> 그림 번호표 (책상 위)
    std::unordered_set<std::string> failed_;  // 이미 실패를 기록한 파일 (같은 오류를 또 적지 않으려고)
    TextureHandle fallback_;
    ResourceStats stats_;
};

}  // namespace wind::engine
