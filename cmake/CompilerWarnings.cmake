# 우리 코드 타깃에만 경고 정책을 적용한다.
# FetchContent로 받은 외부 라이브러리는 자체 컴파일 옵션을 그대로 쓴다.
function(wind_set_warnings target)
    if(MSVC)
        # /W4: 높은 경고 수준, /permissive-: 표준 준수 모드, /utf-8: 소스·실행 문자셋 UTF-8
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()
