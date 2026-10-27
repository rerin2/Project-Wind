# Project WIND — Claude Instructions

## Project
Build an original Windows-first 2D tile-based MMORPG using C++20, CMake, SDL3, ENet, and PostgreSQL. Do not use Unity, Unreal, Godot, or another full game engine. SDL3 and ENet are allowed low-level libraries; implement the game engine systems in this repository.

The full master specification lives in `docs/MASTER_SPEC.md`. Follow its phases (section 21) in order.

## Source of truth
- `docs/MASTER_SPEC.md`: integrated master specification
- `docs/REQUIREMENTS.md`: product requirements
- `docs/ARCHITECTURE.md`: module boundaries
- `docs/ENGINE.md`: custom engine design
- `docs/PROTOCOL.md`: network wire format
- `docs/DATABASE.md`: schema and migrations
- `docs/GAMEPLAY.md`: gameplay rules
- `docs/SECURITY.md`: trust boundaries and validation
- `docs/TESTING.md`: test policy
- `docs/DECISIONS/`: accepted architectural decisions

If a document does not exist yet, create or update it as part of the relevant task. Do not silently invent conflicting rules.

## Mandatory workflow
1. Inspect the repository and read relevant docs before editing.
2. State the small task, files likely to change, and acceptance criteria.
3. Implement only that task.
4. Build the affected targets.
5. Run relevant tests.
6. Inspect the diff for unrelated changes and security issues.
7. Update docs when interfaces, protocol, schema, or behavior changes.
8. Report commands actually run and their real results.

## Architecture rules
- Server is authoritative for movement, combat, HP, XP, inventory, currency, quest rewards, and permissions.
- Client is presentation/input; never trust client-provided outcomes.
- `shared` must not depend on `client` or `server`.
- Server code must not depend on SDL or rendering.
- Do not send raw C++ struct memory over the network. Use explicit serialization.
- All untrusted network input must be length-checked and validated.
- Important inventory/reward operations must be idempotent or transactionally protected.
- Use migrations for database schema changes.
- Keep content data separate from gameplay code and validate it before loading.

## C++ rules
- C++20.
- Prefer RAII, explicit ownership, small functions, and clear interfaces.
- Avoid global mutable state and unexplained magic numbers.
- Do not swallow errors.
- Do not add dependencies without recording version, license, and rationale.
- Do not commit secrets or real credentials.
- Do not disable or delete tests just to get a green build.

## Scope control
Do not attempt to implement the whole MMORPG in one task. Follow the phases in the master specification. Prefer a small working vertical slice over broad placeholder code.

## Completion report
Include:
- Task
- Files changed
- Behavior implemented
- Interface/schema changes
- Build command and result
- Tests run and result
- Known limitations
- Next task

Never claim a build or test passed unless it was actually run and passed.
