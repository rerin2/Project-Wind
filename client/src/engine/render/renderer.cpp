// 이 파일은 무엇을 하나요?
// 화가(Renderer)가 SDL3 붓으로 실제로 그림을 그리는 방법을 적은 곳이에요.
// 게임 전체에서 SDL 그리기 함수를 부르는 곳은 거의 여기뿐이에요.

#include "engine/render/renderer.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

#include <string>

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
}

bool Renderer::set_logical_size(std::int32_t width, std::int32_t height) {
    // INTEGER_SCALE: 1배, 2배, 3배처럼 정수 배로만 키워요. 1.5배처럼 키우면 픽셀 크기가 들쭉날쭉해져요.
    if (!SDL_SetRenderLogicalPresentation(renderer_, width, height,
                                          SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
        log::error("SDL_SetRenderLogicalPresentation failed: {}", SDL_GetError());
        return false;
    }
    logical_size_ = {static_cast<float>(width), static_cast<float>(height)};
    return true;
}

void Renderer::begin_frame(Color clear_color) {
    stats_ = {};  // 이번 화면 통계를 0부터 다시 세요
    set_draw_color(clear_color);
    SDL_RenderClear(renderer_);  // 도화지 전체를 한 색으로 칠하기
}

void Renderer::end_frame() {
    // 지금까지 보이지 않는 뒷면 도화지에 그렸어요. 이제 앞면으로 휙 뒤집어 보여 줘요.
    // (그리는 중간 모습이 보이면 깜빡거려서 이렇게 해요)
    SDL_RenderPresent(renderer_);
    last_stats_ = stats_;
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
    SDL_SetTextureAlphaMod(it->second, sprite.alpha);  // 투명도 정하기
    // 스티커 종이의 src 부분을 오려서, 화면의 dst 자리에 (회전·뒤집기 포함) 붙여요.
    SDL_RenderTextureRotated(renderer_, it->second, &src, &dst, sprite.rotation_degrees, nullptr,
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
