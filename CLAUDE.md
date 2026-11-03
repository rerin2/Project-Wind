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

## Scope control
Do not attempt to implement the whole game in one task. Follow the phases in the master specification. Prefer a small working, good-looking vertical slice over broad placeholder code.

## Completion report
Include:
- Task
- Files changed
- Behavior implemented
- Interface/schema changes
- Build command and result
- Tests run and result
- On-screen check
- Known limitations
- Next task

Never claim a build or test passed unless it was actually run and passed.
