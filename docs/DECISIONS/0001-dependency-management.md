# 0001 — Dependency management: CMake FetchContent

- Status: accepted
- Date: 2026-10-27
- Spec reference: master spec 2.1, 2.2

## Decision

Use CMake `FetchContent` for every third-party library, pinned to an exact Git tag. Do not use vcpkg.

## Reasons

- One tool: CMake already drives the build, so no second package manager has to be installed or bootstrapped.
- Pins live in one file ([cmake/Dependencies.cmake](../../cmake/Dependencies.cmake)) next to the build, reviewable in a diff.
- All dependencies (SDL3, ENet, spdlog, GoogleTest) ship their own CMake builds.

## Costs

- The first configure needs network access and takes longer (SDL3 is the largest).
- ENet's CMakeLists predates CMake 3.5, so `CMAKE_POLICY_VERSION_MINIMUM` is set for it, and its include path and Windows libraries are added by us.

## Revisit when

PostgreSQL client libraries (libpq) are added in phase 4: libpq does not build cleanly through FetchContent. Options then are a system install found with `find_package(PostgreSQL)` or switching to vcpkg; record that as a new decision.
