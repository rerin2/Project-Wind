# 0002 — Test framework: GoogleTest

- Status: accepted
- Date: 2026-10-27
- Spec reference: master spec 2.1 ("GoogleTest or Catch2, fix one early")

## Decision

Use GoogleTest (`v1.17.0`) for all unit and integration tests, registered with CTest via `gtest_discover_tests`.

## Reasons

- First-class CMake/CTest integration; each test case appears individually in `ctest` output.
- Widely used, stable, BSD-3-Clause.

## Conventions

- Each module owns its tests: `shared/tests`, `server/tests`, `client/tests`.
- Process-level smoke tests (running a real executable) are added with `add_test` alongside.
- Tests must not open windows; GUI checks are done manually or with `--max-frames` outside CTest.
