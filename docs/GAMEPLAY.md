# Gameplay Rules

Status: phase 6. Spec references: master spec sections 7–9.

This file holds the rules the simulation enforces and the numbers behind them. Change a number here and in code together.

## Movement

Code: `client/src/game/entities/mover.*`, `movement_tuning.hpp`, `client/src/game/physics/tile_collision.*`.

- The simulation runs at a fixed **60 Hz**. Positions are float world pixels; the player's position is the **feet** point.
- Input is an 8-way unit vector. Diagonals are normalized, so diagonal speed equals straight speed (tested).
- Each step, velocity moves toward `input × max_speed` by at most `acceleration × dt` (or `deceleration × dt` when there is no input).
- **Collision body:** a feet box 14 × 8 px whose bottom-centre is the feet point. The sprite does not collide; only the feet box does.
- **Tiles:** a tile blocks if its walkability is false (from the tileset, see ENGINE.md). Outside the map is blocked.
- **Wall sliding:** movement is applied in sub-steps of ≤ 1 px, x first, then y. A blocked axis stops and its velocity becomes 0; the other axis continues.
- **Corner assist:** when moving along a single axis and blocked, the body is nudged sideways by up to the step length if an opening is found within **6 px**. Diagonal movement does not use corner assist.
- **Facing:** follows the held direction (not the velocity), picked from 4 directions by the dominant axis; an exact diagonal keeps the current facing when it is one of the two components.

| Tuning | Value | Why |
|---|---|---|
| `max_speed` | 110 px/s (≈ 3.4 tiles/s) | Brisk but readable on a 640×360 view (≈ 20 tiles wide) |
| `acceleration` | 1100 px/s² | Reaches full speed in ≈ 0.1 s: responsive without feeling weightless |
| `deceleration` | 1400 px/s² | Stops in ≈ 0.08 s, a little faster than starting, so stopping feels precise |
| `corner_assist` | 6 px | Covers small misalignment when entering 1-tile gaps |
| feet box | 14 × 8 px | Narrower than a 32 px tile so 1-tile paths are easy to enter |

## Dodge

- Pressing dodge records it in the input buffer for **0.15 s**. A dodge starts on the first step where it is allowed.
- Direction: the held direction, or the facing direction when nothing is held.
- During a dodge, velocity is fixed at `dodge_direction × dodge_speed`; walls still stop it.
- When it ends, velocity becomes `dodge_direction × max_speed` (no sudden stop) and a cooldown starts.
- The first part of a dodge is invulnerable (`Mover::invulnerable`), used by combat in phase 12 (player survival).

| Tuning | Value | Why |
|---|---|---|
| `dodge_speed` | 280 px/s | ≈ 2.5× walking speed |
| `dodge_seconds` | 0.20 s | Covers ≈ 56 px (a bit under 2 tiles) |
| `dodge_cooldown_seconds` | 0.30 s | Prevents chaining dodges into fast travel |
| `dodge_invulnerable_seconds` | 0.16 s | Most of the roll; the tail end is vulnerable |
| input buffer | 0.15 s | Spec 5.8 default |

## Time

- The game clock runs at 60 real seconds per game hour (a game day is 24 real minutes), starting at 09:00 for a new game. It advances in the fixed simulation step and continues across map travel. Saving the time comes with saves (phase 16).
- Time and weather are presentation only for now: nothing in the rules depends on them yet.

## Not yet defined

Combat (phases 9–12), growth and items (13–15), and saves (16) are defined later (spec section 19).
