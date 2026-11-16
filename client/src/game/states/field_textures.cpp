// 이 파일은 무엇을 하나요?
// 그림자·파티클·빛 그림을 코드로 만들고, 필드가 끝나면 지우는 곳이에요.

#include "game/states/field_textures.hpp"

#include "engine/render/renderer.hpp"

namespace wind::game {
namespace {

// 발밑 그림자 크기와 진하기 (가운데가 가장 진하고 가장자리는 투명해요)
constexpr std::int32_t kShadowWidth = 22;
constexpr std::int32_t kShadowHeight = 8;
constexpr engine::Color kShadowColor{0, 0, 0, 110};
// 파티클 그림: 흰색이라 효과 파일의 색을 곱해서 써요.
constexpr std::int32_t kSoftParticleSize = 8;
constexpr engine::Color kWhite{255, 255, 255, 255};
// 빛 그림 크기 (확대해서 쓰니까 너무 크지 않아도 돼요)
constexpr std::int32_t kLightTextureSize = 128;

}  // namespace

FieldTextures::FieldTextures(engine::Renderer& renderer)
    : renderer_(&renderer),
      shadow_(renderer.create_soft_ellipse_texture(kShadowWidth, kShadowHeight, kShadowColor)),  // 그림자
      particles_{renderer.create_soft_ellipse_texture(kSoftParticleSize, kSoftParticleSize, kWhite),  // 부드러운 점
                 renderer.create_solid_texture(1, 1, kWhite)},                                      // 1픽셀 점
      light_(renderer.create_light_texture(kLightTextureSize)) {}                                    // 빛

FieldTextures::~FieldTextures() {
    // 파일에서 읽은 그림은 리소스 관리자가 정리하고, 여기서는 코드로 만든 것만 지워요.
    for (const engine::TextureHandle texture : {shadow_, particles_.soft, particles_.pixel, light_}) {
        renderer_->destroy_texture(texture);
    }
}

engine::Vec2 FieldTextures::shadow_size() const {
    return {static_cast<float>(kShadowWidth), static_cast<float>(kShadowHeight)};  // 만든 크기 그대로
}

}  // namespace wind::game
