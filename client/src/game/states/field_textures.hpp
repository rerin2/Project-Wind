#pragma once

// 이 파일은 무엇을 하나요?
// 필드 화면이 파일 없이 코드로 만드는 그림들을 맡아요. (field_state.cpp에서 나눠 왔어요)
// - 발밑 그림자 (부드러운 검은 타원)
// - 파티클 그림 (부드러운 흰 점, 흰 1픽셀)
// - 빛 그림 (가운데가 밝은 흰 동그라미)
// - 피격 비네트 그림 (가운데 투명, 가장자리가 진한 흰색, 11c)
// 만들 때 그리고(생성자), 필드가 사라질 때 지워요(소멸자). 그래서 지우는 걸 잊을 수 없어요(RAII).
// 만들기에 실패한 그림은 빈 번호표라서, 그 효과만 안 보이고 게임은 계속돼요.

#include "engine/fx/particle_drawing.hpp"
#include "engine/core/math.hpp"
#include "engine/render/render_types.hpp"

namespace wind::engine {
class Renderer;
}

namespace wind::game {

class FieldTextures {
public:
    // renderer: 그림을 만들고 지울 화가 (이 물건보다 오래 살아 있어야 해요)
    explicit FieldTextures(engine::Renderer& renderer);
    ~FieldTextures();
    FieldTextures(const FieldTextures&) = delete;             // 같은 그림을 두 번 지우지 않게 복사 금지
    FieldTextures& operator=(const FieldTextures&) = delete;

    [[nodiscard]] engine::TextureHandle shadow() const { return shadow_; }                 // 발밑 그림자
    [[nodiscard]] engine::Vec2 shadow_size() const;                                         // 그림자 그림 크기 (픽셀)
    [[nodiscard]] const engine::ParticleTextures& particles() const { return particles_; }  // 파티클 그림
    [[nodiscard]] engine::TextureHandle light() const { return light_; }                   // 빛 그림
    [[nodiscard]] engine::TextureHandle vignette() const { return vignette_; }             // 피격 비네트 그림

private:
    engine::Renderer* renderer_;           // 빌려 쓰는 화가
    engine::TextureHandle shadow_;         // 발밑 그림자
    engine::ParticleTextures particles_;   // 부드러운 점, 1픽셀 점
    engine::TextureHandle light_;          // 둥근 빛
    engine::TextureHandle vignette_;       // 피격 비네트
};

}  // namespace wind::game
