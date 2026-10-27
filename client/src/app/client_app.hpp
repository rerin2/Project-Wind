#pragma once

// 클라이언트 최상위 루프.

#include "app/client_options.hpp"

#include <cstdint>

namespace wind::client {

// 단계 0에서는 창을 열고 매 프레임 화면을 지우기만 한다.
// 명세 5.1의 전체 프레임 순서(입력·네트워크·애니메이션·렌더링 등)는 단계 1에서 채운다.
class ClientApp {
public:
    explicit ClientApp(ClientOptions options);

    // 창이 닫히거나 max_frames에 도달할 때까지 실행한다.
    // 반환값은 프로세스 종료 코드다 (정상 종료 0, 초기화 실패 1).
    int run();

private:
    ClientOptions options_;
    std::uint64_t frames_rendered_ = 0;  // 지금까지 그린 프레임 수
};

}  // namespace wind::client
