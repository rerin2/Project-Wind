#pragma once

// 이 파일은 무엇을 하나요?
// 게임 클라이언트(플레이어가 켜는 게임 프로그램)의 "맨 위 반복"을 맡은 설계도예요.
// 게임은 영화 필름처럼 그림을 1초에 약 60장 빠르게 바꿔서 움직여 보이게 해요.
// 그 한 장 한 장을 만들 때마다 같은 순서로 일을 해요. (명세 5.1)
//   1) 창에서 일어난 일 확인 (창 닫기 등)  2) 키보드 상태 확인
//   3) (네트워크 받기: 단계 2에서 추가)    4) 정해진 박자로 게임 규칙 계산
//   5) 그림 넘기기·카메라 움직이기         6) 화면 그리기
//   7) (소리: 나중 단계)                  8) 디버그 정보 그리기

#include "app/client_options.hpp"

#include <cstdint>

namespace wind::client {

class ClientApp {
public:
    // 실행 옵션(창 크기, 맵 파일 등)을 받아서 준비해요.
    explicit ClientApp(ClientOptions options);

    // 게임을 실행해요. 창을 닫거나, Esc를 누르거나, --max-frames 장수를 다 그리면 끝나요.
    // 돌려주는 숫자는 "종료 코드"예요. 0 = 잘 끝남, 1 = 준비하다 실패함
    int run();

private:
    ClientOptions options_;
    std::uint64_t frames_rendered_ = 0;  // 지금까지 그린 화면 장수
};

}  // namespace wind::client
