# MusicSurfer Incremental Implementation Plan

This plan follows the requested order: plan first, then scaffold validation, then incremental feature implementation with compile checks after each step.

## Step 1 — Planning and baseline validation

### Goals
- Confirm current repository structure aligns with the requested architecture.
- Define phased implementation milestones for MVP functionality.
- Define compile/test gates per phase.

### Deliverables
- This implementation plan.
- Baseline build and test run.

### Exit criteria
- `cmake` configure succeeds.
- Core targets build.
- Existing tests pass.

---

## Step 2 — Scaffold and architecture alignment

### Goals
- Ensure folders and module boundaries match the target structure:
  - `core`, `services`, `audio`, `ui`, `database`, `utils`.
- Ensure CMake cleanly composes modules behind a single library target.
- Ensure no hardcoded paths and no global mutable state.

### Deliverables
- Any required scaffold fixes (headers/sources/CMake).
- Updated README architecture section if needed.

### Compile gate
- Reconfigure + full build after scaffold updates.

---

## Step 3 — Core domain model hardening

### Goals
- Finalize domain types:
  - `Track`, `Album`, `Artist`, `Playlist`, identifiers.
- Keep domain layer persistence-agnostic.

### Deliverables
- Documented public methods.
- Validation for required fields (id/path where applicable).

### Compile/test gate
- Build succeeds.
- Unit tests for model invariants pass.

---

## Step 4 — Infrastructure: scanner + metadata + database

### Goals
- `FileSystemScanner`: recursive directory scanning and extension filtering (`.mp3`, `.flac`, `.wav`).
- `MetadataParser`: extract title/artist/album/duration (TagLib-backed when enabled; safe fallback otherwise).
- `Database`: SQLite wrapper with schema management and prepared statements.

### Deliverables
- Schema creation for:
  - `Tracks(id, title, artist, album, path, duration)`
  - `Playlists(id, name)`
  - `PlaylistTracks(playlist_id, track_id)`
- CRUD methods needed by service layer.

### Compile/test gate
- Build succeeds with and without optional TagLib/JUCE flags.
- Tests covering scan, parse, and DB persistence pass.

---

## Step 5 — Services: LibraryManager, AudioPlayer, PlaylistManager

### Goals
- `LibraryManager`: scan, parse, and sync tracks into repository/DB.
- `AudioPlayer`: play/pause/stop/seek/load abstraction; buffering-ready interface.
- `PlaylistManager`: create/delete playlists and add/remove tracks with persistence.

### Deliverables
- Service interfaces and concrete implementations.
- Dependency injection through interfaces to keep coupling low.

### Compile/test gate
- Build succeeds.
- Service-level tests pass for sync/playback state machine/playlist persistence.

---

## Step 6 — JUCE UI integration (MVP)

### Goals
- Build modular main window with:
  - Track list panel
  - Playback controls
  - Playlist panel
- Wire UI to service layer using MVC/MVVM-like boundaries.

### Deliverables
- Main window component and panel components.
- Command handlers for transport and playlist actions.

### Compile/test gate
- Build succeeds with JUCE enabled.
- Basic smoke tests for service bindings pass.

---

## Step 7 — End-to-end flow and polish

### Goals
- Connect startup flow:
  - open DB
  - initialize managers
  - load library and playlists
- Improve logging and error surfacing for recoverable failures.

### Deliverables
- Functional sample `main.cpp` end-to-end path.
- Updated README with build options and known limitations.

### Compile/test gate
- Full build + `ctest` pass.

---

## Step 8 — Extensibility readiness

### Goals
- Prepare interfaces for future features:
  - search/filter/sort
  - artwork
  - drag & drop playlists
  - equalizer/replay gain

### Deliverables
- Extension points in service and UI abstractions.
- Backlog notes for post-MVP phases.

### Compile/test gate
- Build remains green after refactoring.

---

## Quality and coding constraints enforced in every phase

- C++17+ features with RAII and smart pointers.
- No global mutable variables.
- Small focused functions and clear interfaces.
- Avoid hardcoded paths; use configurable roots.
- Unit tests added with each module increment.
