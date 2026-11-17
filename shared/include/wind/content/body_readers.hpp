#pragma once

// 이 파일은 무엇을 하나요?
// 허수아비 파일과 몬스터 파일이 함께 쓰는 "몸 칸 읽기" 도우미 함수 모음이에요. (9c에서 만들고 11a에서 나눔)
// - "hurtbox": { "offset": [x, y], "size": [w, h] }        맞는 상자
// - "sprite": { "image": "...", "frame": [w, h], "origin": [x, y] }  그림 띠
// 틀린 칸은 JsonReader의 오류 공책에 적고 nullopt를 돌려줘요. 범위는 docs/ENGINE.md에 있어요.

#include "wind/combat/hurtbox_shape.hpp"
#include "wind/combat/sprite_strip.hpp"
#include "wind/content/json_reader.hpp"

#include <optional>

namespace wind::content {

// "hurtbox" 칸 (발에서 -128~128픽셀, 크기 1~128픽셀)
[[nodiscard]] std::optional<combat::HurtboxShape> read_hurtbox(JsonReader& reader);

// "sprite" 칸 (칸 크기 1~128픽셀, 발 위치는 칸 안)
[[nodiscard]] std::optional<combat::SpriteStrip> read_sprite_strip(JsonReader& reader);

}  // namespace wind::content
