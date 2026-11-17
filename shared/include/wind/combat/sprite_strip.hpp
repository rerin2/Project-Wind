#pragma once

// 이 파일은 무엇을 하나요?
// 한 줄로 늘어선 그림 칸 띠(스프라이트 띠) 설명이에요. 허수아비(9c)와 몬스터(11a)가 함께 써요.
// - 허수아비: 0 = 서 있음, 1 = 쓰러짐
// - 몬스터:   0 = 보통, 1 = 공격 예고(웅크림)
// 칸은 모두 같은 크기이고, 그림 속 "발 위치"(origin)가 맵의 발 위치에 오게 그려요.

#include <string>

namespace wind::combat {

struct SpriteStrip {
    std::string image;        // 그림 파일 (콘텐츠 폴더 기준)
    int frame_width = 32;     // 한 칸 가로
    int frame_height = 48;    // 한 칸 세로
    int origin_x = 16;        // 그림 속 발 위치 (가로)
    int origin_y = 44;        // 그림 속 발 위치 (세로)
};

}  // namespace wind::combat
