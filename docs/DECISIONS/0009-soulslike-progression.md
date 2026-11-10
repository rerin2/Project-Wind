# 0009 — Soulslike progression: runes, stat allocation, weapon weight, sorceries

- Status: accepted (design; implementation in phases 8, 9, 13, 15)
- Date: 2026-11-10
- Spec reference: master spec 7.2.1, 8.1–8.7, 9.5, 11.3, 12, 13.2, 19 (phases 9, 13, 15, 16, 18), 23.2
- Requested by: the developer (soulslike levelling, three weapon types, glintstone-style sorceries as reference)

## Decision

Replace the experience/level-up design with soulslike progression.

1. **Runes instead of experience.** Monsters, chests and quests give runes. Runes are both the level-up cost and the shop currency (no separate gold).
2. **Level up only at save points** ("catnip cushions": rest restores HP, MP, stamina and status, sets the respawn point, respawns monsters and saves the game; see spec 8.7). The player picks which stat to raise; each level adds 1 to one stat. Confirming is all-or-nothing.
3. **Five stats, 1–60 each:** vigor (max HP), mind (max MP), endurance (max stamina), strength (physical attack scaling, carry weight), intelligence (magic attack, spell requirements, spell slots). Level = sum of stats − 49 (start 1, max 251). Only the five stats are saved; everything else is computed by one function from data curves with soft caps.
4. **Level cost** `floor(a + b·(L−1) + c·(L−1)²)` with data coefficients a = 100, b = 40, c = 6. This is our own curve, not copied from another game.
5. **Death** loses `floor(runes / 2)`. Level, stats and items are kept. Lost runes cannot be recovered (initial rule). The game saves right after the loss so reloading cannot undo it.
6. **Weapons** (dagger 2, longsword 6, greatsword 14 weight) differ in base power, strength scaling, timing, combo length and stamina per swing. A weapon heavier than the strength-based carry weight can be equipped but is "heavy": attack timeline ×1.3, attack stamina ×1.5. Attacks cost stamina.
7. **Sorceries ("starlight sorcery")**: star shard (basic bolt), great star shard (slow heavy bolt), star swarm (orbiting homing stars), moon blade (melee arc), comet beam (held beam, MP per second). Each has an intelligence requirement; spell slots (2–4) grow with intelligence; slots change at cushions.

## Why

- The developer wants players to choose how the hero grows (HP, MP, stamina, strength, intelligence) instead of fixed level gains.
- Losing half the runes gives death a cost without wiping progress; a recoverable bloodstain is left as an open decision (23.2) to try after playtesting.
- Weight on strength makes the greatsword a goal you level towards, which suits the "손맛" (game feel) target.

## Originality

Mechanics are inspired by the soulslike genre and by popular sorcery archetypes (basic bolt, heavy bolt, homing orbs, magic blade, beam). Names, numbers, formulas and the setting are our own; no names, assets or formulas from other games are used.

## Consequences

- Spec 8 rewritten; 9.5 death, 11.3 shop currency, 12 save contents, 13 HUD (rune counter, no XP bar), 14 data examples updated.
- New content folders: `content/progression/` (stat curves, level cost), `content/weapons/`, `content/spells/`. Maps gain `save_point` entities (catnip cushions).
- Monster data uses `rune_reward`; monsters reappear when the player rests or respawns, instead of on a timer.
- New states: `casting`, `resting`, `exhausted`.

## Open

- Rune recovery at the death spot, poise, a dexterity stat for daggers, catalysts (staffs) that change magic power.
