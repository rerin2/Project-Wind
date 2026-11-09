#pragma once

// 이 파일은 무엇을 하나요?
// 렌더러 = 실제로 화면에 그림을 그리는 "화가"예요. (명세 5.2)
// 속에서는 SDL3(그림 도구 상자)의 붓을 쓰지만, 게임 코드에는 우리 엔진의 재료만 보여 줘요.
// 게임 화면은 항상 640x360 크기로 그리고, 창이 크면 2배, 3배처럼 "딱 떨어지는 배수"로
// 키워요. 그래야 도트 그림(픽셀 아트)이 뭉개지지 않고 또렷해요.

#include "engine/render/render_queue.hpp"
#include "engine/render/render_types.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

// SDL의 속 모습은 숨기고 이름만 알려 줘요. (그래서 이 헤더를 쓰는 쪽은 SDL을 몰라도 돼요)
struct SDL_Renderer;
struct SDL_Texture;

namespace wind::engine {

class Renderer {
public:
    // SDL 붓(SDL_Renderer)을 빌려서 써요. 붓의 주인은 SdlPlatform이라 여기서 버리지 않아요.
    explicit Renderer(SDL_Renderer* renderer);
    // 화가가 그만둘 때, 자기가 만든 스티커 종이(텍스처)는 모두 정리해요.
    ~Renderer();
    // 화가는 한 명뿐이라 복사할 수 없어요.
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // 게임 화면 크기(논리 해상도)를 정하고, 그 크기의 작은 도화지(렌더 타깃)를 만들어요.
    // 매 화면은 작은 도화지에 먼저 그리고, 끝나면 창에 정수 배로 키워 붙여요. 남는 곳은 검게 둬요.
    bool set_logical_size(std::int32_t width, std::int32_t height);
    [[nodiscard]] Vec2 logical_size() const { return logical_size_; }

    // 새 그림 시작: 도화지를 한 가지 색으로 싹 칠해요.
    void begin_frame(Color clear_color);
    // 그림 완성: 다 그린 도화지를 화면에 짠 하고 보여 줘요.
    void end_frame();

    // 줄 세운 순서표(렌더 큐)를 위에서부터 차례로 그려요. (먼저 queue.sort()를 해야 해요)
    void submit(const RenderQueue& queue);
    // 그림 조각 하나 그리기
    void draw_sprite(const SpriteDraw& sprite);
    // 색칠한 네모 하나 그리기
    void fill_rect(RectF rect, Color color);
    // 8x8 크기의 작은 글자로 영어·숫자(ASCII) 글을 써요. (디버그 화면용)
    void draw_debug_text(Vec2 position, std::string_view text, Color color);
    static constexpr float kDebugCharSize = 8.0f;  // 글자 한 개의 크기(픽셀)

    // BMP 그림 파일을 읽어서 스티커 종이(텍스처)로 만들어요.
    // 분홍색(마젠타, 빨강 255 + 파랑 255)은 "투명"으로 처리해요. 크로마키처럼요.
    // 실패하면 빈 번호표를 돌려주고, error에 이유를 적어요.
    [[nodiscard]] TextureHandle load_bmp_texture(const std::filesystem::path& path, std::string& error);
    // 그림 파일이 없을 때 대신 쓰는 분홍/검정 체크무늬 스티커를 만들어요.
    [[nodiscard]] TextureHandle create_checker_texture(std::int32_t size);
    // 가운데는 진하고 가장자리로 갈수록 흐려지는 타원 스티커를 만들어요.
    // 발밑 그림자(검은색)나 먼지(밝은 흙색)처럼 부드러운 모양에 써요.
    // color.a는 가운데의 가장 진한 투명도예요.
    [[nodiscard]] TextureHandle create_soft_ellipse_texture(std::int32_t width, std::int32_t height,
                                                            Color color);
    // 스티커 종이 하나를 버려요.
    void destroy_texture(TextureHandle handle);
    // 지금 가진 스티커 종이 수
    [[nodiscard]] std::size_t texture_count() const { return textures_.size(); }

    // 바로 전 화면을 그릴 때의 통계 (디버그 화면용)
    [[nodiscard]] const RenderStats& last_frame_stats() const { return last_stats_; }

    // 스크린샷 예약: 이번 화면을 다 그리면(end_frame) path에 BMP 그림으로 저장해요.
    // 저장 결과는 기록(로그)에 남겨요. 실패해도 게임은 계속돼요.
    void request_screenshot(std::filesystem::path path);

private:
    // 새 스티커 종이에 번호표를 붙여 보관함에 넣어요.
    TextureHandle register_texture(SDL_Texture* texture);
    // 붓의 색을 바꿔요.
    void set_draw_color(Color color);
    // 예약된 스크린샷이 있으면 지금 그려진 화면을 저장해요.
    void save_pending_screenshot();

    SDL_Renderer* renderer_;  // 빌려 쓰는 SDL 붓
    SDL_Texture* frame_target_ = nullptr;  // 게임 화면을 먼저 그리는 640x360 작은 도화지
    std::unordered_map<std::uint32_t, SDL_Texture*> textures_;  // 번호표 -> 스티커 종이 보관함
    std::uint32_t next_texture_id_ = 1;  // 다음에 줄 번호표 (0은 "없음"이라 1부터)
    Vec2 logical_size_;
    RenderStats stats_;       // 지금 그리고 있는 화면의 통계
    RenderStats last_stats_;  // 바로 전 화면의 통계
    std::optional<std::filesystem::path> pending_screenshot_;  // 저장 예약된 스크린샷 위치
};

}  // namespace wind::engine
