# 0003 — Logging: wind::log over spdlog

- Status: accepted
- Date: 2026-10-27
- Spec reference: master spec 2.1, 18.3

## Decision

All code logs through `wind::log` ([shared/include/wind/log.hpp](../../shared/include/wind/log.hpp)). spdlog `v1.15.3` is the backend.

Levels follow spec 18.3: `trace`, `debug`, `info`, `warn`, `error`, `critical` (plus `off`). Each process sets a component name (`server`, `client`) shown in every line. Warnings and above flush immediately.

## Reasons

- spdlog is a lightweight, widely used, MIT-licensed library with format-string checking at compile time.
- The thin wrapper keeps spdlog out of call sites' vocabulary, so it can be replaced later.

## Not yet done

- Structured (JSON) output, log rotation, and retention policy (spec 18.3) — phase 7.
- Rule: never log passwords, session secrets, or tokens.
