# Project WIND

An original Windows-first 2D pixel-art single-player action RPG with a custom engine, written in C++20 on SDL3. Visual quality (lighting, particles, weather, game feel) is a first-class goal.

All game content and branding are original. See [docs/MASTER_SPEC.md](docs/MASTER_SPEC.md) for the full specification and [CLAUDE.md](CLAUDE.md) for contributor rules.

## Status

**Phase 11 — monsters.** Two green slimes live in the meadow below the training dummies (walk down from the start). They rest, breathing slowly, and hop around their home spot. Come within about four tiles and one hops in surprise with a yellow "!", then hops after you. Close up it stops, crouches and shivers while a red path glows on the ground where it will land (a red light keeps it visible at night) — step out of the red area before it leaps. Run far enough away and it gives up and hops home. F3 shows the busiest slime state (`mobs 2 (chase)`); F4 shows hurtboxes, feet boxes and the lunge hitbox. If the lunge lands you lose HP (shown on F3 as `hp 60/100`; the slime data now hits for 40), freeze for a moment, get pushed back (sliding along walls) and blink while you cannot be hit again; the screen edges flash red. Rolling at the right moment dodges the lunge, and slimes and the heroine push each other apart instead of overlapping. Strike back with the sword (J) or the star shard (L): the slime flashes white, gets pushed back and staggers (a hit while it crouches cancels its leap), and an HP bar appears above it. At 30 HP it falls on the third sword hit, melts flat while blinking and bursts into mint droplets. Leave the meadow and come back and the slimes are there again.

**Phase 10 — hit feel.** A landed hit freezes the action for a few frames (hitstop), the dummy flashes white, its body is pushed toward the hit and springs back, gold sparks fly, and a damage number pops up above its HP bar; the combo finisher shakes the screen. 5 % of hits are critical for 1.5× damage, shown as a bold gold number with "!". `--shake 0` and `--flash 0` turn shaking and flashing off (any value from 0 to 1 scales them).

**Phase 9 — sword, training dummies and the first spell.** Press **J** (or Z) to swing the longsword; pressing again in the right moment chains a 3-hit combo (14 → 15 → 21 damage), and each swing costs 14 stamina. Two straw training dummies stand below the start of the meadow path: hit them to see the slash arc and their HP bar drop; at 0 HP they tip over and stand up again after 1.5 s. F4 shows the sword hitbox and the dummies' hurtboxes. Press **L** (or C) to cast the star shard: a spark gathers at her hand for 0.25 s (you can still walk slowly), then a glittering shard flies 240 px, deals 12 damage to the first dummy it touches and vanishes at walls. Each cast costs 6 of the 50 MP shown by the blue bar under the stamina bar; MP refills at 2 per second, and when it runs short the bar blinks red and nothing is cast. At night the shard lights up the grass as it flies.

**Phase 8 — text, UI and stamina.** Korean text with the Galmuri11 pixel font: an area name banner slides in when you enter a map (시험 초원, 시험 동굴, 시험 숲), and the F7 UI test panel shows 9-slice windows, wrapped and clipped text and buttons you pick with ↑/↓ and Enter. Hold **Shift** to run (165 px/s); running and dodging use stamina, shown by the bar at the top left, and running dry leaves the heroine exhausted until it refills to 25.

**Phase 7 — living world and travel.** Water shimmers, flowers lean, leaves and tall grass sway in a wave of wind. The right end of the meadow path fades into a test forest; the cave mouth uses an iris (a circle closing on the heroine). A broken exit keeps you on the current map and shows a notice.

**Phase 6 — time and weather.** The meadow goes from day to sunset to night (1 real minute = 1 game hour); lanterns and fireflies come out at night; rain, snow and fog come and go. F5 skips 3 hours, F6 changes the weather, `--time 21 --weather rain` starts at a chosen moment.

**Phase 5 — lighting.** A dark test cave with flickering torches, ember sparks, a pulsing blue light and a small light that follows the heroine. From phase 4: petals and fireflies drift over the meadow, and walking and dodging kick up dust; every effect is a JSON file in `content/fx/`. From phase 3: the game draws into a 640×360 frame target, with blend modes, tweens and F12 screenshots. From phase 2: the client shows a test map where the hero walks in 8 directions with acceleration, slides along walls, and dodge-rolls with afterimages and dust. The camera looks slightly ahead. Engine design: [docs/ENGINE.md](docs/ENGINE.md). Movement rules: [docs/GAMEPLAY.md](docs/GAMEPLAY.md). Effect values: [docs/VISUALS.md](docs/VISUALS.md).

To reach the cave, walk left along the dirt path from the start into the dark arch in the wall; the arch inside the cave leads back. Walk right to the end of the path to reach the forest.

Controls: arrow keys / WASD move, **Shift** run, **J** / **Z** attack, **Space** (or K / X) dodge, **F3** debug overlay, **F4** collision overlay, **F5** time, **F6** weather, **F7** UI test panel, **F12** screenshot, **Esc** quit.

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
