#pragma once

// 이 파일은 무엇을 하나요?
// 훈련용 허수아비 설명(데이터)이에요. (명세 9단계 "HP를 가진 훈련용 허수아비")
// 숫자는 content/props/training_dummy.json에서 읽어요. 코드에 숫자를 박아 두지 않아요.

#include <string>

namespace wind::combat {

// 맞는 상자(허트박스): 허수아비 발 위치를 기준으로 왼쪽 위 모서리와 크기 (픽셀)
struct HurtboxShape {
    float x = -10.0f;  // 발에서 왼쪽으로 (음수 = 왼쪽)
    float y = -30.0f;  // 발에서 위로 (음수 = 위)
    float w = 20.0f;   // 가로
    float h = 30.0f;   // 세로
};

// 허수아비 그림: 한 줄에 두 칸 (0 = 서 있음, 1 = 쓰러짐)
struct DummySprite {
    std::string image;        // 그림 파일 (콘텐츠 폴더 기준)
    int frame_width = 32;     // 한 칸 가로
    int frame_height = 48;    // 한 칸 세로
    int origin_x = 16;        // 그림 속 발 위치 (가로)
    int origin_y = 44;        // 그림 속 발 위치 (세로)
};

struct TrainingDummyData {
    std::string id;                // "training_dummy"
    std::string name;              // "훈련용 허수아비"
    int max_hp = 100;              // 가득 찬 HP
    float defense = 0.0f;          // 방어력 (피해 계산 9.3)
    HurtboxShape hurtbox;          // 맞는 상자
    float recover_seconds = 1.5f;  // HP가 0이 되면 쓰러져 있다가 다시 서기까지 (초)
    DummySprite sprite;            // 그림
};

}  // namespace wind::combat
