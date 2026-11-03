# 0007 — Single-player action RPG with visual focus (replaces the MMORPG plan)

- Status: accepted
- Date: 2026-11-03
- Spec reference: master spec 2.0 (all sections), replaces spec 1.1
- Supersedes: 0004 (server tick rate); the ENet part of 0001

## Decision

- The game is a **single-player** 2D pixel-art action RPG. There is no game server, networking, account system, or database.
- Movement changes from one-tile steps to **free 8-direction pixel movement** with a small feet collider, wall sliding, and corner correction.
- Combat is **real-time action** (attack windup/active/recovery, hitboxes, dodge) instead of classic auto-attack.
- **Visual quality is a first-class goal**: pixel art plus lighting, particles, day/night, weather, game feel (hitstop, flash, knockback, shake, damage numbers), screen transitions, and animated UI.
- Rendering stays on the SDL3 renderer, using blend modes and render targets for lighting. The SDL3 GPU API (shaders) is reconsidered only if those prove insufficient.
- Game state is split into a fixed-step **simulation** (SDL-free, deterministic, testable) and a **presentation** layer driven by simulation state and events.
- Progress is saved to local JSON files with atomic writes, a backup, and versioned migrations.

## Consequences

- Phase 2 removes `server/`, the ENet dependency, `scripts/run_server.ps1`, and the server tests and docs.
- `GridMover` (phase 1) is replaced by free movement in phase 2. The tile map, camera, render queue, animation, input, and resource code from phase 1 stay.
- Authority/anti-cheat rules no longer apply. Content and save files are still treated as untrusted input.
- The new phase plan (spec section 19) is: 2 cleanup + free movement, 3 visual foundation, 4 real-time combat, 5 growth/items/UI, 6 save/title/audio, 7 world and story, 8 polish and release.

## Alternatives considered

- **HD-2D style post-processing** (bloom, depth of field) — prettiest, but needs a shader pipeline from the start. Deferred.
- **High-resolution illustrated art** — highest art cost for a small team. Rejected.
- **Keep tile movement** — simpler and keeps existing code, but less smooth for action combat. Rejected.
- **Classic RPG combat** — less work, but weaker game feel. Rejected.
