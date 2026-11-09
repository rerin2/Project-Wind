// 이 파일은 무엇을 하나요?
// 화가(Renderer)가 SDL3 붓으로 실제로 그림을 그리는 방법을 적은 곳이에요.
// 게임 전체에서 SDL 그리기 함수를 부르는 곳은 거의 여기뿐이에요.

#include "engine/render/renderer.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <system_error>
#include <utility>

namespace wind::engine {
namespace {

// 우리 엔진의 "뒤집기" 값을 SDL이 알아듣는 값으로 바꿔요.
SDL_FlipMode to_sdl(Flip flip) {
    switch (flip) {
        case Flip::Horizontal: return SDL_FLIP_HORIZONTAL;
        case Flip::Vertical: return SDL_FLIP_VERTICAL;
        case Flip::None: break;
    }
    return SDL_FLIP_NONE;
}

// 우리 엔진의 "섞기 방법"을 SDL이 알아듣는 값으로 바꿔요.
// Multiply는 MUL을 써요: 투명한 부분은 아래 그림을 그대로 두고, 진한 부분만 곱해서 어둡게 해요.
SDL_BlendMode to_sdl(BlendMode blend) {
    switch (blend) {
        case BlendMode::Add: return SDL_BLENDMODE_ADD;
        case BlendMode::Multiply: return SDL_BLENDMODE_MUL;
        case BlendMode::Normal: break;
    }
    return SDL_BLENDMODE_BLEND;
}

}  // namespace

Renderer::Renderer(SDL_Renderer* renderer) : renderer_(renderer) {
    // 그림을 키울 때 "가장 가까운 픽셀 색"을 그대로 써요 (최근접 이웃 방식).
    // 이렇게 하면 도트 그림이 흐릿해지지 않고 네모난 픽셀 그대로 커져요. (명세 5.4)
    if (!SDL_SetDefaultTextureScaleMode(renderer_, SDL_SCALEMODE_NEAREST)) {
        log::warn("Cannot set nearest-neighbour scaling: {}", SDL_GetError());
    }
    // 반투명 색(충돌 표시의 빨간 네모, 디버그 판의 검은 배경)이 뒤가 비쳐 보이게 섞기 모드를 켜요.
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
}

Renderer::~Renderer() {
    // 보관함에 있는 스티커 종이를 모두 버려요.
    for (const auto& [id, texture] : textures_) {
        SDL_DestroyTexture(texture);
    }
    if (frame_target_ != nullptr) {
        SDL_DestroyTexture(frame_target_);
    }
}

bool Renderer::set_logical_size(std::int32_t width, std::int32_t height) {
    // INTEGER_SCALE: 1배, 2배, 3배처럼 정수 배로만 키워요. 1.5배처럼 키우면 픽셀 크기가 들쭉날쭉해져요.
    // 이 설정은 마지막에 "작은 도화지"를 창에 붙일 때만 쓰여요.
    if (!SDL_SetRenderLogicalPresentation(renderer_, width, height,
                                          SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
        log::error("SDL_SetRenderLogicalPresentation failed: {}", SDL_GetError());
        return false;
    }
    // 작은 도화지(렌더 타깃) = 640x360 크기의 그림 종이. 게임 화면은 모두 여기에 먼저 그려요.
    // 나중에 조명·화면 효과를 이 종이 위에 덧칠한 다음, 통째로 창에 크게 붙일 거예요. (명세 5.2)
    if (frame_target_ != nullptr) {
        SDL_DestroyTexture(frame_target_);
    }
    frame_target_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET, width, height);
    if (frame_target_ == nullptr) {
        log::error("Cannot create the {}x{} frame target: {}", width, height, SDL_GetError());
        return false;
    }
    SDL_SetTextureScaleMode(frame_target_, SDL_SCALEMODE_NEAREST);  // 키울 때 또렷하게
    SDL_SetTextureBlendMode(frame_target_, SDL_BLENDMODE_NONE);     // 창에 붙일 때 그대로 덮어쓰기
    logical_size_ = {static_cast<float>(width), static_cast<float>(height)};
    return true;
}

void Renderer::begin_frame(Color clear_color) {
    stats_ = {};  // 이번 화면 통계를 0부터 다시 세요
    // 이번 화면은 창이 아니라 작은 도화지에 그려요.
    if (!SDL_SetRenderTarget(renderer_, frame_target_)) {
        log::error("Cannot draw into the frame target: {}", SDL_GetError());
    }
    set_draw_color(clear_color);
    SDL_RenderClear(renderer_);  // 도화지 전체를 한 색으로 칠하기
}

void Renderer::end_frame() {
    // 화면에 보여 주기 전에 스크린샷부터 찍어요. 작은 도화지 그대로(640x360) 저장돼요.
    save_pending_screenshot();
    // 작은 도화지를 창에 정수 배로 크게 붙여요. 남는 곳은 검은 띠로 채워요.
    SDL_SetRenderTarget(renderer_, nullptr);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, frame_target_, nullptr, nullptr);
    // 지금까지 보이지 않는 뒷면 도화지에 그렸어요. 이제 앞면으로 휙 뒤집어 보여 줘요.
    // (그리는 중간 모습이 보이면 깜빡거려서 이렇게 해요)
    SDL_RenderPresent(renderer_);
    last_stats_ = stats_;
}

void Renderer::request_screenshot(std::filesystem::path path) {
    pending_screenshot_ = std::move(path);
}

void Renderer::save_pending_screenshot() {
    if (!pending_screenshot_) {
        return;
    }
    const std::filesystem::path path = std::move(*pending_screenshot_);
    pending_screenshot_.reset();

    // 저장할 폴더가 없으면 만들어요.
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        log::error("Cannot create screenshot folder {}: {}", path.parent_path().string(), ec.message());
        return;
    }
    // 지금 도화지의 픽셀을 그래픽 카드에서 읽어 와요.
    SDL_Surface* surface = SDL_RenderReadPixels(renderer_, nullptr);
    if (surface == nullptr) {
        log::error("Cannot read pixels for screenshot: {}", SDL_GetError());
        return;
    }
    const auto utf8 = path.u8string();
    if (SDL_SaveBMP(surface, reinterpret_cast<const char*>(utf8.c_str()))) {
        log::info("Saved screenshot {} ({}x{})", path.string(), surface->w, surface->h);
    } else {
        log::error("Cannot save screenshot {}: {}", path.string(), SDL_GetError());
    }
    SDL_DestroySurface(surface);
}

// 순서표에 적힌 대로 하나씩 그려요.
void Renderer::submit(const RenderQueue& queue) {
    for (const DrawCommand& cmd : queue.commands()) {
        if (cmd.kind == DrawCommand::Kind::Sprite) {
            draw_sprite(cmd.sprite);
        } else {
            fill_rect(cmd.rect, cmd.color);
        }
    }
}

void Renderer::draw_sprite(const SpriteDraw& sprite) {
    // 화면 밖에 붙일 그림은 그리지 않고 건너뛰어요. (컬링 = 안 보이는 것 솎아 내기)
    // 안 보이는 걸 그리면 그래픽 카드만 힘들어요.
    const RectF screen{0.0f, 0.0f, logical_size_.x, logical_size_.y};
    if (!sprite.dest.intersects(screen)) {
        ++stats_.culled;
        return;
    }
    // 번호표로 보관함에서 스티커 종이를 찾아요.
    const auto it = textures_.find(sprite.texture.id);
    if (it == textures_.end()) {
        return;  // 없는 번호표. 리소스 관리자가 대신 그림을 주니까 보통은 일어나지 않아요.
    }

    // 우리 엔진의 네모를 SDL의 네모 모양으로 옮겨 적어요.
    const SDL_FRect src{static_cast<float>(sprite.source.x), static_cast<float>(sprite.source.y),
                        static_cast<float>(sprite.source.w), static_cast<float>(sprite.source.h)};
    const SDL_FRect dst{sprite.dest.x, sprite.dest.y, sprite.dest.w, sprite.dest.h};
    // 색 곱하기, 투명도, 섞기 방법을 정해요. (스티커 종이마다 기억되므로 매번 다시 정해요)
    SDL_SetTextureColorMod(it->second, sprite.tint.r, sprite.tint.g, sprite.tint.b);
    SDL_SetTextureAlphaMod(it->second, sprite.tint.a);
    SDL_SetTextureBlendMode(it->second, to_sdl(sprite.blend));
    // 스티커 종이의 src 부분을 오려서, 화면의 dst 자리에 (회전·뒤집기 포함) 붙여요.
    // source 크기가 0이면 "그림 전체"를 써요 (부드러운 동그라미처럼 통째로 쓰는 그림).
    const bool whole_texture = sprite.source.w <= 0 || sprite.source.h <= 0;
    SDL_RenderTextureRotated(renderer_, it->second, whole_texture ? nullptr : &src, &dst,
                             sprite.rotation_degrees, nullptr,
                             to_sdl(sprite.flip));
    ++stats_.draw_calls;
}

void Renderer::fill_rect(RectF rect, Color color) {
    set_draw_color(color);
    const SDL_FRect r{rect.x, rect.y, rect.w, rect.h};
    SDL_RenderFillRect(renderer_, &r);
    ++stats_.draw_calls;
}

void Renderer::draw_debug_text(Vec2 position, std::string_view text, Color color) {
    set_draw_color(color);
    // SDL은 글자 끝에 "끝 표시(0)"가 붙은 글자만 받아서, 끝 표시가 있는 복사본을 만들어 넘겨요.
    const std::string terminated(text);
    SDL_RenderDebugText(renderer_, position.x, position.y, terminated.c_str());
    ++stats_.draw_calls;
}

TextureHandle Renderer::load_bmp_texture(const std::filesystem::path& path, std::string& error) {
    // SDL은 UTF-8 글자로 된 파일 위치를 받아요. (한국어 폴더 이름도 문제없게)
    const auto utf8 = path.u8string();
    // 1) 파일을 읽어서 "서피스"(컴퓨터 기억 속의 그림)로 만들어요.
    SDL_Surface* surface = SDL_LoadBMP(reinterpret_cast<const char*>(utf8.c_str()));
    if (surface == nullptr) {
        error = SDL_GetError();
        return {};
    }
    // 2) 분홍색(255,0,255) 픽셀은 투명하게 처리해요. (컬러 키, 규칙: docs/ENGINE.md)
    //    BMP 파일은 투명색을 직접 담기 어려워서, "이 색은 없는 셈 쳐 줘"라고 약속한 거예요.
    SDL_SetSurfaceColorKey(surface, true, SDL_MapSurfaceRGB(surface, 255, 0, 255));
    // 3) 그래픽 카드에 올려서 빠르게 그릴 수 있는 텍스처(스티커 종이)로 바꿔요.
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);  // 기억 속 그림은 이제 필요 없어서 버려요
    if (texture == nullptr) {
        error = SDL_GetError();
        return {};
    }
    return register_texture(texture);
}

TextureHandle Renderer::create_checker_texture(std::int32_t size) {
    // 빈 그림을 하나 만들어요.
    SDL_Surface* surface = SDL_CreateSurface(size, size, SDL_PIXELFORMAT_RGBA8888);
    if (surface == nullptr) {
        log::error("Cannot create placeholder surface: {}", SDL_GetError());
        return {};
    }
    // 4x4칸 분홍/검정 체크무늬를 칠해요. 그림이 빠진 곳이 화면에서 바로 눈에 띄도록요.
    const std::int32_t cell = size / 4;
    for (std::int32_t y = 0; y < 4; ++y) {
        for (std::int32_t x = 0; x < 4; ++x) {
            const SDL_Rect r{x * cell, y * cell, cell, cell};
            const bool magenta = ((x + y) % 2) == 0;  // 바둑판처럼 번갈아 칠하기
            SDL_FillSurfaceRect(surface, &r,
                                SDL_MapSurfaceRGB(surface, magenta ? 255 : 0, 0, magenta ? 255 : 0));
        }
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);
    if (texture == nullptr) {
        log::error("Cannot create placeholder texture: {}", SDL_GetError());
        return {};
    }
    return register_texture(texture);
}

TextureHandle Renderer::create_soft_ellipse_texture(std::int32_t width, std::int32_t height,
                                                    Color color) {
    if (width <= 0 || height <= 0) {
        log::error("Soft ellipse size must be positive, got {}x{}", width, height);
        return {};
    }
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (surface == nullptr) {
        log::error("Cannot create soft ellipse surface: {}", SDL_GetError());
        return {};
    }
    // 픽셀마다 "타원 가운데에서 얼마나 떨어졌나"(0 = 가운데, 1 = 가장자리)를 재서
    // 가운데는 진하게, 가장자리로 갈수록 투명하게 칠해요.
    const float half_w = static_cast<float>(width) * 0.5f;
    const float half_h = static_cast<float>(height) * 0.5f;
    auto* pixels = static_cast<std::uint8_t*>(surface->pixels);
    for (std::int32_t y = 0; y < height; ++y) {
        std::uint8_t* row = pixels + static_cast<std::ptrdiff_t>(y) * surface->pitch;
        for (std::int32_t x = 0; x < width; ++x) {
            const float dx = (static_cast<float>(x) + 0.5f - half_w) / half_w;
            const float dy = (static_cast<float>(y) + 0.5f - half_h) / half_h;
            const float distance = std::sqrt(dx * dx + dy * dy);
            const float strength = std::clamp(1.0f - distance, 0.0f, 1.0f);
            // 제곱근으로 가장자리를 살짝 넓혀서, 너무 작은 점처럼 보이지 않게 해요.
            const float alpha = std::sqrt(strength) * static_cast<float>(color.a);
            std::uint8_t* px = row + static_cast<std::ptrdiff_t>(x) * 4;
            px[0] = color.r;
            px[1] = color.g;
            px[2] = color.b;
            px[3] = static_cast<std::uint8_t>(alpha);
        }
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_DestroySurface(surface);
    if (texture == nullptr) {
        log::error("Cannot create soft ellipse texture: {}", SDL_GetError());
        return {};
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);  // 투명한 부분이 뒤가 비쳐 보이게
    return register_texture(texture);
}

void Renderer::destroy_texture(TextureHandle handle) {
    const auto it = textures_.find(handle.id);
    if (it == textures_.end()) {
        return;  // 이미 없는 번호표면 할 일 없음
    }
    SDL_DestroyTexture(it->second);
    textures_.erase(it);
}

TextureHandle Renderer::register_texture(SDL_Texture* texture) {
    // 이 스티커도 키울 때 흐려지지 않게 "가장 가까운 픽셀" 방식으로 정해요.
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    const std::uint32_t id = next_texture_id_++;
    textures_.emplace(id, texture);
    return {id};
}

void Renderer::set_draw_color(Color color) {
    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
}

}  // namespace wind::engine
