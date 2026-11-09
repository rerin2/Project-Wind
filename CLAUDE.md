# Project WIND — Claude Instructions

## Project
Build an original Windows-first 2D pixel-art single-player action RPG using C++20, CMake, and SDL3. Do not use Unity, Unreal, Godot, or another full game engine. SDL3 is an allowed low-level library; implement the game engine systems in this repository. Visual quality (lighting, particles, weather, day/night, game feel, transitions, UI animation) is a first-class goal.

The full master specification lives in `docs/MASTER_SPEC.md`. Follow its phases (section 19) in order.

## Source of truth
- `docs/MASTER_SPEC.md`: integrated master specification
- `docs/ENGINE.md`: custom engine design and content formats
- `docs/VISUALS.md`: art rules, palette, effect tuning
- `docs/GAMEPLAY.md`: gameplay rules
- `docs/SAVE_FORMAT.md`: save file format and migrations
- `docs/TESTING.md`: test policy
- `docs/DECISIONS/`: accepted architectural decisions

If a document does not exist yet, create or update it as part of the relevant task. Do not silently invent conflicting rules.

## Mandatory workflow
1. Inspect the repository and read relevant docs before editing.
2. State the small task, files likely to change, and acceptance criteria.
3. Implement only that task.
4. Build the affected targets.
5. Run relevant tests.
6. For visible features, run the client and check the result on screen.
7. Inspect the diff for unrelated changes.
8. Update docs when interfaces, content formats, save format, or behavior change.
9. Report commands actually run and their real results.

## Architecture rules
- Game simulation (movement, collision, combat, HP, XP, inventory, quests, save data) runs at a fixed step and must not depend on SDL or rendering.
- Presentation (rendering, particles, lighting, audio, UI) reads simulation state and events; it must not change simulation outcomes. Hitstop is passed to the simulation explicitly as a time scale.
- `shared` must not depend on `client`.
- SDL calls live only in the engine platform/render/resources/audio layers.
- Content and save files are untrusted input: validate size, ranges, and references; confine paths to the content root or save folder; never crash on bad files.
- Saves are written atomically with a backup, carry a schema version, and are migrated by tested functions.
- Inventory, shop, and quest-reward operations are all-or-nothing.
- Keep content data and effect tuning values separate from gameplay code.

## C++ rules
- C++20.
- Prefer RAII, explicit ownership, small functions, and clear interfaces.
- Avoid global mutable state and unexplained magic numbers.
- Do not swallow errors.
- Do not add dependencies without recording version, license, and rationale.
- Do not disable or delete tests just to get a green build.

## Module and file rules (spec 2.4)
- One class per file pair: `class Foo` (or a `struct` with member functions) lives in `foo.hpp` + `foo.cpp`, named in snake_case.
- A small enum or plain data struct used only by that class may stay in its file; once another class uses it, move it to its own file.
- Exceptions: a header of tiny value types on one topic (e.g. `math.hpp`), and one file pair of free helper functions per topic (e.g. `coordinates.hpp`).
- One class does one job. If its purpose needs an "and", split it. Review any `.cpp` over ~300 lines for splitting.
- Folder = module. Keep related classes in one folder and follow the dependency direction; engine code never includes game code.
- Keep headers light: include only what is needed, forward-declare types used by pointer or reference, and include SDL headers only in SDL-layer `.cpp` files.
- Tests for `Foo` go in `foo_tests.cpp`.
- Split existing files that break these rules when you next touch them, or in a separate behavior-preserving cleanup task verified by build and tests.

## Scope control
Do not attempt to implement the whole game in one task. Follow the phases in the master specification. Prefer a small working, good-looking vertical slice over broad placeholder code.

코드를 수정하거나 추가 할 때, 13살도 이해할 수 있게 간결하게 설명

# 개발 작업 완료 보고 규칙

모든 개발 작업이 끝나면 다음 내용을 보고한다.

## 1. 작업 요약
- 구현한 기능
- 구현 목적
- 요구사항 충족 여부

## 2. 변경된 파일
- 생성한 파일
- 수정한 파일
- 삭제한 파일
- 각 파일의 변경 목적

## 3. 시스템 구조 영향
- 기존 시스템 중 영향을 받은 부분
- 새롭게 연결된 모듈
- 변경된 데이터 흐름
- 기존 인터페이스에 미친 영향

## 4. 핵심 로직 설명
- 기능의 전체 실행 흐름
- 주요 클래스와 함수의 역할
- 중요한 알고리즘
- 상태 변경 과정
- 예외 및 오류 처리 방식

## 5. 설계 변경 사항
- 기존 설계와 달라진 부분
- 변경이 필요한 이유
- 관련 설계 문서 업데이트 여부

## 6. 테스트 결과
- 빌드 성공 여부
- 실행 테스트 결과
- 단위 테스트 결과
- 통합 테스트 결과
- 화면 확인 결과 (눈에 보이는 기능)
- 발견된 문제와 미해결 항목

실제로 실행해서 통과하지 않은 빌드나 테스트를 통과했다고 보고하지 않는다.

## 7. 개발자 확인 필요 사항
- 사람이 반드시 검토해야 할 코드
- 보안 및 안정성 관련 위험
- 성능상 주의 사항
- 추가적인 기술적 의사결정이 필요한 사항

## 8. 다음 작업 제안
- 다음 개발 단계
- 선행되어야 하는 작업
- 기존 구현에서 개선할 부분
