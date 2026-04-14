# MusicSurfer (Starter Layout)

This repository contains a minimal CMake-based C++ project scaffold for a future MusicSurfer app.

## Requirements

- CMake 3.20+
- A C++17-compatible compiler (MSVC, GCC, or Clang)

Optional dependencies (enable with CMake options):

- **JUCE** (`MUSIC_SURFER_ENABLE_JUCE=ON`) – requires a CMake-configured JUCE install.
- **TagLib** (`MUSIC_SURFER_ENABLE_TAGLIB=ON`) – requires TagLib with CMake package config support.
- **SQLite** (`MUSIC_SURFER_ENABLE_SQLITE=ON`) – requires SQLite3 development package.

Optional quality-of-life CMake options:

- **Sanitizers** (`MUSIC_SURFER_ENABLE_SANITIZERS=ON`) – enables AddressSanitizer + UBSanitizer for Debug builds on GCC/Clang.

Build hygiene:

- In-source builds are blocked (use `-B build`).
- `compile_commands.json` is exported for tooling (`CMAKE_EXPORT_COMPILE_COMMANDS=ON`).

## Build (Linux)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```


## Build with CMake Presets (recommended on Linux)

```bash
cmake --preset linux-release
cmake --build --preset build-release
ctest --preset test-release
```

Available presets:
- `linux-debug` / `build-debug` / `test-debug`
- `linux-release` / `build-release` / `test-release`
- `linux-asan` / `build-asan` / `test-asan`

## Build (Windows, Developer PowerShell)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Notes

- Main app target: `music_surfer_app`
- Test target: `music_surfer_tests` (when `MUSIC_SURFER_BUILD_TESTS=ON`)
- Core module library: `music_surfer_core`
- CMake is split by scope: top-level orchestrates, `src/CMakeLists.txt` defines app/core targets, and `tests/CMakeLists.txt` defines test targets.

Build organization:

- Top-level `CMakeLists.txt` configures global options and delegates to `src/` and `tests/` subdirectories.
- `src/CMakeLists.txt` defines app + core targets.
- `tests/CMakeLists.txt` defines all unit/integration test targets.

## Milestone build logs

Clean configure/build (and where relevant test) outputs for Milestones A-F are recorded under:

- `build_logs/milestone_A.log`
- `build_logs/milestone_B.log`
- `build_logs/milestone_C.log`
- `build_logs/milestone_D.log`
- `build_logs/milestone_E.log`
- `build_logs/milestone_F.log`
