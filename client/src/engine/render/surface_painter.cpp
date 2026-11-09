// 이 파일은 무엇을 하나요?
// 코드로 만드는 그림들을 픽셀 단위로 칠하는 곳이에요.

#include "engine/render/surface_painter.hpp"

#include "wind/log.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace wind::engine {
namespace {

// RGBA 빈 서피스를 만들어요. 크기가 이상하거나 만들 수 없으면 이유를 기록하고 nullptr.
SDL_Surface* blank(std::int32_t width, std::int32_t height, const char* what) {
    if (width <= 0 || height <= 0) {
        log::error("{} size must be positive, got {}x{}", what, width, height);
        return nullptr;
    }
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (surface == nullptr) {
        log::error("Cannot create {} surface: {}", what, SDL_GetError());
    }
    return surface;
}

// 픽셀 하나에 색을 칠해요. (RGBA32 = 빨강, 초록, 파랑, 진하기 순서로 한 바이트씩)
void put(SDL_Surface* surface, std::int32_t x, std::int32_t y, Color c) {
    auto* row = static_cast<std::uint8_t*>(surface->pixels) + static_cast<std::ptrdiff_t>(y) * surface->pitch;
    std::uint8_t* px = row + static_cast<std::ptrdiff_t>(x) * 4;
    px[0] = c.r;
    px[1] = c.g;
    px[2] = c.b;
    px[3] = c.a;
}

// (x, y) 픽셀이 가운데에서 얼마나 떨어졌는지: 가운데 0, 가장자리 1 (가로·세로를 따로 맞춰요)
float distance_from_center(std::int32_t x, std::int32_t y, float half_w, float half_h) {
    const float dx = (static_cast<float>(x) + 0.5f - half_w) / half_w;
    const float dy = (static_cast<float>(y) + 0.5f - half_h) / half_h;
    return std::sqrt(dx * dx + dy * dy);
}

}  // namespace

SDL_Surface* paint_checker(std::int32_t size) {
    SDL_Surface* surface = blank(size, size, "placeholder");
    if (surface == nullptr) {
        return nullptr;
    }
    // 4x4칸 분홍/검정 체크무늬를 칠해요. 그림이 빠진 곳이 화면에서 바로 눈에 띄도록요.
    const std::int32_t cell = std::max(1, size / 4);
    for (std::int32_t y = 0; y < size; ++y) {
        for (std::int32_t x = 0; x < size; ++x) {
            const bool magenta = ((x / cell + y / cell) % 2) == 0;  // 바둑판처럼 번갈아 칠하기
            put(surface, x, y, magenta ? Color{255, 0, 255, 255} : Color{0, 0, 0, 255});
        }
    }
    return surface;
}

SDL_Surface* paint_soft_ellipse(std::int32_t width, std::int32_t height, Color color) {
    SDL_Surface* surface = blank(width, height, "soft ellipse");
    if (surface == nullptr) {
        return nullptr;
    }
    const float half_w = static_cast<float>(width) * 0.5f;
    const float half_h = static_cast<float>(height) * 0.5f;
    for (std::int32_t y = 0; y < height; ++y) {
        for (std::int32_t x = 0; x < width; ++x) {
            const float strength = std::clamp(1.0f - distance_from_center(x, y, half_w, half_h), 0.0f, 1.0f);
            // 제곱근으로 가장자리를 살짝 넓혀서, 너무 작은 점처럼 보이지 않게 해요.
            const float alpha = std::sqrt(strength) * static_cast<float>(color.a);
            put(surface, x, y, {color.r, color.g, color.b, static_cast<std::uint8_t>(alpha)});
        }
    }
    return surface;
}

SDL_Surface* paint_radial_light(std::int32_t size) {
    SDL_Surface* surface = blank(size, size, "light");
    if (surface == nullptr) {
        return nullptr;
    }
    const float half = static_cast<float>(size) * 0.5f;
    for (std::int32_t y = 0; y < size; ++y) {
        for (std::int32_t x = 0; x < size; ++x) {
            const float d = distance_from_center(x, y, half, half);
            const float d2 = std::min(1.0f, d * d);
            // (1 - 거리²)²: 가운데는 넓게 밝고, 가장자리는 부드럽게 사라지는 모양이에요.
            const float falloff = (1.0f - d2) * (1.0f - d2);
            put(surface, x, y, {255, 255, 255, static_cast<std::uint8_t>(falloff * 255.0f)});
        }
    }
    return surface;
}

SDL_Surface* paint_solid(std::int32_t width, std::int32_t height, Color color) {
    SDL_Surface* surface = blank(width, height, "solid");
    if (surface == nullptr) {
        return nullptr;
    }
    for (std::int32_t y = 0; y < height; ++y) {
        for (std::int32_t x = 0; x < width; ++x) {
            put(surface, x, y, color);
        }
    }
    return surface;
}

}  // namespace wind::engine
