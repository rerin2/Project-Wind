# Project WIND

An original Windows-first 2D pixel-art single-player action RPG with a custom engine, written in C++20 on SDL3. Visual quality (lighting, particles, weather, game feel) is a first-class goal.

All game content and branding are original. See [docs/MASTER_SPEC.md](docs/MASTER_SPEC.md) for the full specification and [CLAUDE.md](CLAUDE.md) for contributor rules.

## Status

**Phase 5 — lighting.** A dark test cave with flickering torches, ember sparks, a pulsing blue light and a small light that follows the heroine. From phase 4: petals and fireflies drift over the meadow, and walking and dodging kick up dust; every effect is a JSON file in `content/fx/`. From phase 3: the game draws into a 640×360 frame target, with blend modes, tweens and F12 screenshots. From phase 2: the client shows a test map where the hero walks in 8 directions with acceleration, slides along walls, and dodge-rolls with afterimages and dust. The camera looks slightly ahead. Engine design: [docs/ENGINE.md](docs/ENGINE.md). Movement rules: [docs/GAMEPLAY.md](docs/GAMEPLAY.md). Effect values: [docs/VISUALS.md](docs/VISUALS.md).

To reach the cave, walk left along the dirt path from the start into the dark arch in the wall; the arch inside the cave leads back.

Controls: arrow keys / WASD move, **Space** (or K / X) dodge, **F3** debug overlay, **F4** collision overlay, **F12** screenshot, **Esc** quit.

## Quick start (Windows)

Requires Visual Studio 2022 Build Tools (C++ workload), CMake ≥ 3.25, Ninja, and Git. Full setup: [docs/BUILD.md](docs/BUILD.md).

```powershell
.\scripts\build.ps1          # configure + build (Debug)
.\scripts\run_tests.ps1      # run all tests
.\scripts\run_client.ps1     # open the client window
```

## Layout

```text
shared/   SDL-free code (logging, CLI, map model, content validation)
client/   the game: engine/ (custom engine), game/ (simulation and effects), app/ (loop, overlay)
content/  game data: maps, tilesets, sprite sheets, particle effects (JSON + BMP)
tools/    content generators and (later) validators
cmake/    dependency pins and compiler settings
scripts/  build/run/test PowerShell scripts
docs/     specification, build guide, decisions
```

## Dependencies

Pinned versions and licenses: [DEPENDENCIES.md](DEPENDENCIES.md).
