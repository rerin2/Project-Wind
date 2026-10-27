# 0004 — Server tick: 20 Hz, fixed step, no catch-up

- Status: accepted
- Date: 2026-10-27
- Spec reference: master spec 3.4, 25.2

## Decision

- The server runs one logical world loop at a fixed rate, default **20 Hz**, configurable with `--tick-rate` (1–120).
- When a tick runs late, the loop resumes from the current time instead of running catch-up ticks. Overruns are counted and logged at `warn`.
- Tick duration is measured every tick so the rate can be revisited with data.

## Reasons

The spec sets 20 Hz as the first default and asks that it be configurable and measured. Skipping catch-up avoids a spiral where a slow tick causes a burst of more ticks.
