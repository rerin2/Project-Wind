# Project WIND

An original Windows-first 2D tile-based MMORPG with a custom engine, written in C++20.

- **Client:** custom engine on SDL3 (window, input, rendering backend, audio)
- **Server:** authoritative game server on ENet (UDP transport), single world loop
- **Database:** PostgreSQL (from phase 4)

All game content and branding are original. See [docs/MASTER_SPEC.md](docs/MASTER_SPEC.md) for the full specification and [CLAUDE.md](CLAUDE.md) for contributor rules.

## Status

**Phase 1 — minimal engine.** The client shows a test map and a character you can walk around locally (tile movement, collision, camera follow, animation, debug overlay). The server runs an empty 20 Hz tick loop. No networking yet — client and server are not connected until phase 2. Engine design: [docs/ENGINE.md](docs/ENGINE.md).

Client controls: arrow keys / WASD move, **F3** debug overlay, **F4** collision overlay, **Esc** quit.

## Quick start (Windows)

Requires Visual Studio 2022 Build Tools (C++ workload), CMake ≥ 3.25, Ninja, and Git. Full setup: [docs/BUILD.md](docs/BUILD.md).

```powershell
.\scripts\build.ps1          # configure + build (Debug)
.\scripts\run_tests.ps1      # run all tests
.\scripts\run_server.ps1     # start the server (Ctrl+C to stop)
.\scripts\run_client.ps1     # open the client window
```

## Layout

```text
shared/   platform-independent code used by client and server (logging, CLI, map model, content validation)
server/   authoritative game server (no SDL dependency)
client/   game client: engine/ (custom engine), game/ (gameplay presentation), app/ (loop, overlay)
content/  game data: maps, tilesets, sprite sheets (JSON + BMP)
tools/    content generators and (later) validators
cmake/    dependency pins and compiler settings
scripts/  build/run/test PowerShell scripts
docs/     specification, build guide, decisions
```

## Dependencies

Pinned versions and licenses: [DEPENDENCIES.md](DEPENDENCIES.md).
