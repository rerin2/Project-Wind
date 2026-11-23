# 0010 — Rune recovery at the death spot ("rune trace")

- Status: accepted (design; implementation planned as phase 14d)
- Date: 2026-11-23
- Spec reference: master spec 8.3, 8.3.1, 9.5, 12.1, 12.3, 13.1, 18, 19 (phase 14), 23.2
- Requested by: the developer
- Supersedes: point 5 of `0009-soulslike-progression.md` ("Lost runes cannot be recovered (initial rule)")

## Decision

When the hero falls, the runes she loses (`floor(runes / 2)`, unchanged) are left at the spot where she fell as a **rune trace** (룬 흔적).

1. The trace holds exactly the lost amount, at the hero's feet (always a walkable spot). No trace is made when 0 runes are lost.
2. There is at most one trace in the whole game. It stays on its map through respawn, resting, map travel and monster respawns.
3. Walking within 24 px of it (alive, not resting) recovers the whole amount at once (clamped to the wallet cap) and removes it. It can be recovered only once.
4. Falling again before recovering it removes the old trace for good and leaves a new one with the newly lost runes at the new spot. The two amounts are not added. The old trace is removed even if the new loss is 0.
5. The trace (map, position, amount) is part of `HeroProgress` and the save file. The save written on death covers losing runes, removing the old trace and creating the new one in one write. Recovering is not saved separately; quitting before the next save restores the trace and the unrecovered wallet together, so runes are never doubled.

## Why

- The developer wants death to give a second chance: a risky walk back to get the runes, which suits the soulslike loop the game is built on.
- One trace and "die again and it is gone" keep the cost of dying real without stacking losses forever.
- Automatic recovery on touch matches item pickup (phase 14b), so there is no extra key to learn.

## Alternatives considered

- Press **E** to recover (more deliberate, but adds a prompt in the middle of fights). Not chosen; can be revisited after playtesting.
- Keep all old traces (too forgiving, and the map fills with traces).
- Recover only part of the trace (adds a second penalty players find unclear).

## Originality

A recoverable death drop is a common genre mechanic. The name, look (a slowly swirling gold pool with rising motes), numbers and rules here are our own.

## Consequences

- `HeroProgress` gains an optional trace; `fall()` replaces it. A field shows and recovers the trace only on its own map.
- Fallen screen text gains "쓰러진 자리에 남았다" and, when one was lost, "되찾지 못한 룬 M이 사라졌다".
- Save format (phase 16) must include the trace.
