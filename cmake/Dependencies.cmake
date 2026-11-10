# 외부 라이브러리. configure 시점에 내려받고 정확한 태그로 버전을 고정한다.
# 여기를 바꾸면 DEPENDENCIES.md(버전·라이선스·도입 이유)도 함께 갱신한다.
include(FetchContent)

# 다운로드 진행 상황을 출력해 첫 configure가 멈춘 것처럼 보이지 않게 한다.
set(FETCHCONTENT_QUIET OFF)

# --- spdlog: wind::log 뒤에서 동작하는 로그 백엔드 -----------------------------
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)  # 정적 라이브러리로 빌드
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3
    GIT_SHALLOW    TRUE)

# --- nlohmann/json: 콘텐츠 JSON(맵·타일셋·스프라이트) 파싱 --------------------------
# JSON은 { "이름": 값 } 모양으로 적는 글자 설명서예요. 맵과 캐릭터 정보를 이 모양으로 적어요.
# 이 라이브러리(남이 만들어 둔 도구 모음)가 그 글자를 읽어서 프로그램이 쓸 수 있게 바꿔 줘요.
# 라이브러리 자체의 숙제 검사(테스트)는 우리에게 필요 없어서 끄고 받아요.
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(spdlog nlohmann_json)

# --- SDL3: 창·입력·렌더러·오디오 (클라이언트 전용) ---------------------------------
if(WIND_BUILD_CLIENT)
    # 정적 링크로 SDL3.dll 배포가 필요 없게 하고, 테스트·예제 빌드는 끈다.
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.4.18
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(SDL3)

    # --- SDL3_ttf: 글꼴(TTF) 글자 그리기 (단계 8, 한글 픽셀 글꼴 Galmuri11) -----
    # 함께 들어 있는 FreeType만 쓴다(정적). 한글 완성형은 글자 모양 다듬기(HarfBuzz)가
    # 필요 없고, 컬러 이모지(plutosvg)도 쓰지 않아서 끈다. 그래서 FreeType 하위 모듈만 받는다.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
    set(SDLTTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
    set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
    set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
    set(SDLTTF_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(SDL3_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG        release-3.2.2
        GIT_SHALLOW    TRUE
        GIT_SUBMODULES "external/freetype")
    FetchContent_MakeAvailable(SDL3_ttf)
endif()

# --- GoogleTest: 단위 테스트 -------------------------------------------------
if(WIND_BUILD_TESTS)
    # 우리 코드와 같은 동적 CRT(/MD)를 쓰도록 맞춘다. 다르면 링크 오류가 난다.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)  # 아직 mock은 쓰지 않는다
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(googletest)
endif()
