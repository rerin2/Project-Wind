# 0005 — Content formats: JSON data, BMP images, nlohmann/json

- Status: accepted
- Date: 2026-11-02
- Spec reference: master spec 5.5–5.7, 6.1, 16

## Decisions

1. **Data files are JSON** with a `schema_version` field, validated before use. Maps, tilesets and sprite sheets are separate files; maps reference their tileset by content-relative path. Formats are documented in [docs/ENGINE.md](../ENGINE.md).
2. **Map tiles are plain integer arrays** (row-major, one row per line in the file). No compression yet; maps are small (40×30 test map ≈ 16 KB). Revisit when maps grow past a few hundred KB.
3. **Tile ids start at 1**; 0 means empty. Id N is the N-th cell of the tileset image.
4. **Walkability is derived from tile properties**: a cell is walkable when it has at least one tile and every tile on it is walkable. No separate collision layer.
5. **Images are 24-bit BMP with magenta `(255,0,255)` as the transparent colour**, loaded with SDL3's built-in `SDL_LoadBMP`. This avoids adding an image-decoding dependency in phase 1.
6. **JSON parsing uses nlohmann/json v3.12.0** (MIT), fetched with FetchContent. It lives in `shared` so the server can validate the same maps in phase 2.
7. **Content paths are confined to the content root**: absolute paths, drive letters, `..` and control characters are rejected by `wind::content::normalize_content_path`.

## Reasons

- JSON is in the spec (2.1) and is easy to diff and review.
- Integer arrays need no tooling and match the spec's conceptual example (6.1).
- BMP + colour key keeps the dependency list short while art is placeholder-only.
- nlohmann/json is header-only, widely used, MIT-licensed, and supports parsing without exceptions (`allow_exceptions=false`), which we use for untrusted files.

## Consequences

- Real art with soft edges / partial transparency needs PNG + alpha. When that arrives, add an image decoder (e.g. SDL_image or stb_image), record it in `DEPENDENCIES.md`, and write a new decision.
- Large maps may need run-length or chunked storage later.
