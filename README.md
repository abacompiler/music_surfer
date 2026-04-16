# MusicSurfer

MusicSurfer is a CMake-based C++17 application scaffold for a desktop music player.

It currently includes:
- core domain/services (library, playlists, audio-player state)
- SQLite-backed persistence
- a UI layer (`MainWindow`, `TrackListComponent`, `PlaybackControlsComponent`, `PlaylistPanelComponent`) that acts as the visual/presentation interface model for the app

## 1) Prerequisites

Required:
- CMake 3.20+
- A C++17 compiler (MSVC, GCC, or Clang)

Optional dependencies (enable with CMake options):
- **JUCE** (`MUSIC_SURFER_ENABLE_JUCE=ON`) – requires a CMake-configured JUCE install.
- **TagLib** (`MUSIC_SURFER_ENABLE_TAGLIB=ON`) – requires TagLib with CMake package config support.
- **SQLite** (`MUSIC_SURFER_ENABLE_SQLITE=ON`) – requires SQLite3 development package.

Optional build feature:
- **Sanitizers** (`MUSIC_SURFER_ENABLE_SANITIZERS=ON`) – AddressSanitizer + UBSanitizer for Debug builds on GCC/Clang.

Build hygiene:
- In-source builds are blocked (use `-B <build-dir>`).
- `compile_commands.json` is exported for tooling.

---

## 2) Build and run on Windows (recommended workflow)

Use **Developer PowerShell for VS 2022** so MSVC and Windows SDK tools are already on your `PATH`.

### Step A — Configure

```powershell
cmake -S . -B build/windows-vs2022 -G "Visual Studio 17 2022" -A x64
```

If you need optional libraries, pass them during configure. Example:

```powershell
cmake -S . -B build/windows-vs2022 -G "Visual Studio 17 2022" -A x64 `
  -DMUSIC_SURFER_ENABLE_SQLITE=ON `
  -DMUSIC_SURFER_ENABLE_TAGLIB=ON `
  -DMUSIC_SURFER_ENABLE_JUCE=OFF
```

### Step B — Compile

```powershell
cmake --build build/windows-vs2022 --config Release
```

### Step C — Run tests

```powershell
ctest --test-dir build/windows-vs2022 -C Release --output-on-failure
```

### Step D — Run the app

```powershell
.\build\windows-vs2022\src\Release\music_surfer_app.exe
```

You can pass database configuration options when launching:

```powershell
.\build\windows-vs2022\src\Release\music_surfer_app.exe --db-path C:\temp\music_surfer.db
```

or:

```powershell
$env:MUSIC_SURFER_DB_PATH="C:\temp\music_surfer.db"
.\build\windows-vs2022\src\Release\music_surfer_app.exe
```

---

## 3) UI / visual interface details

MusicSurfer already contains a user-facing UI architecture in `src/ui`:
- `MainWindow` composes UI sections.
- `TrackListComponent` shows track list state.
- `PlaybackControlsComponent` shows playback actions/state.
- `PlaylistPanelComponent` shows playlist state.

This is implemented as a presentation/view-model layer that can be connected to a concrete desktop GUI toolkit.

### JUCE-backed visual components (optional)

If you want to wire the UI layer directly to JUCE components on Windows:
1. Install JUCE with CMake package config support.
2. Configure with `-DMUSIC_SURFER_ENABLE_JUCE=ON`.
3. Rebuild the project.

When JUCE is enabled, UI components compile against `juce::Component`-compatible types (`MUSIC_SURFER_HAS_JUCE=1`), which is the intended path for a native visual desktop interface.

---

## 4) Build on Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Linux presets

```bash
cmake --preset linux-release
cmake --build --preset build-release
ctest --preset test-release
```

Available presets:
- `linux-debug` / `build-debug` / `test-debug`
- `linux-release` / `build-release` / `test-release`
- `linux-asan` / `build-asan` / `test-asan`

---

## 5) Database location resolution

At startup, `music_surfer_app` resolves the SQLite database file path in the following order:

1. CLI argument: `--db-path <path>`
2. Environment variable: `MUSIC_SURFER_DB_PATH`
3. App config directory (with `music_surfer.db` appended):
   - CLI argument: `--config-dir <dir>`
   - Environment variable: `MUSIC_SURFER_CONFIG_DIR`
4. Default user-local app data directory (with `MusicSurfer/music_surfer.db` appended):
   - Linux: `$XDG_DATA_HOME` or `~/.local/share`
   - macOS: `~/Library/Application Support`
   - Windows: `%LOCALAPPDATA%` (fallback `%APPDATA%`)

The app creates the selected database directory automatically before opening SQLite.

---

## 6) Project structure notes

- Main app target: `music_surfer_app`
- Test target: `music_surfer_tests` (when `MUSIC_SURFER_BUILD_TESTS=ON`)
- Core module library: `music_surfer_core`

Build organization:
- Top-level `CMakeLists.txt` configures global options and delegates to `src/` and `tests/`.
- `src/CMakeLists.txt` defines app + core targets.
- `tests/CMakeLists.txt` defines unit/integration test targets.

---

## Milestone build logs

Clean configure/build (and where relevant test) outputs for Milestones A–F are recorded under:

- `build_logs/milestone_A.log`
- `build_logs/milestone_B.log`
- `build_logs/milestone_C.log`
- `build_logs/milestone_D.log`
- `build_logs/milestone_E.log`
- `build_logs/milestone_F.log`
