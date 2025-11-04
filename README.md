<p align="center">icon</p>
<h1 align="center">Project Title</h1>

<h6 align="center">This repository is the final project for CpET 140 - Computer Programming 1 </h6>

<h3 align="center">Members</h3>

<div align="center">
    <table style="width:82%; margin:0 auto;">
      <tr>
        <th>Name (Last Name, First Name)</th>
        <th>Username</th>
      </tr>
      <tr>
        <td>DE LUNA, MARK CLARENCE L.</td>
        <td></td>
      </tr>
      <tr>
        <td>GUINA, MATTHEW ANTON C.</td>
        <td></td>
      </tr>
        <tr>
            <td>JUSAY, MARY GRACE P.</td>
            <td></td>
        </tr>
        <tr>
            <td>LITERAL, JOSHUA H.</td>
            <td> <a href="https://github.com/joshua-9402"> joshua-9402 </a> </td>
        </tr>
        <tr>
            <td>MATANGUIHAN, JANN VINCENT A.</td>
            <td></td>
        </tr>
        <tr>
            <td>PAILAS, POL JUSTINE T.</td>
            <td></td>
        </tr>
    </table>
</div>


## Technical Overview

This project is a C++ application that demonstrates a small, modular POS/inventory system with a UI layer and SQLite-backed persistence. It is designed for desktop platforms (Windows, macOS, Linux) and can be adapted to mobile platforms (Android, iOS) with platform-specific toolchains.

### Key facts
- Language: C++17 (or later)
- Build system: CMake
- UI: ImGui (immediate-mode GUI)
- Database: SQLite (single-file, local)
- Target platforms: Desktop (Win/macOS/Linux); cross-compilable to Android/iOS

## Architecture

- main
  - Application entry, initialization, main loop, high-level lifecycle.
- src/lib/UI.cpp
  - ImGui-based window composition, window manager, per-window visibility/focus.
- src/lib/db.cpp / db.h
  - SQLite helpers: open/create DB, execute statements, schema creation utilities.
- src/lib/inventory.cpp
  - Inventory domain logic: item models, CRUD helpers.
- src/lib/pos.cpp
  - POS transaction logic and UI glue.

Modules communicate via simple C++ interfaces; DB functions open/close connections per operation unless a shared handle is used.

## Build & Run (desktop)

### Prerequisites
- C++17 toolchain (gcc/clang/MSVC)
- CMake 3.15+
- sqlite3 development headers
- Optional: fetch/build ImGui and any renderer/backends used

### Common commands (from repo root)
- Configure and build:
  - mkdir -p build && cd build
  - cmake .. -DCMAKE_BUILD_TYPE=Release
  - cmake --build . --config Release
- Run:
  - ./your-executable-name

### Mobile (notes)
- Android: use Android NDK and an Android CMake toolchain file; build an APK or use native-lib JNI entry.
- iOS: generate an Xcode project with CMake or build with appropriate SDK flags; run in Simulator/Device.

## Dependencies

- sqlite3 (runtime + development headers)
- ImGui (library or submodule; integrate with your renderer)
- Optional: libraries for platform windows/rendering (GLFW/SDL/DirectX/Metal)

## Testing & Validation

- Manual smoke tests: startup, DB create/open, add item, create sale, close and reopen DB, UI interactions.
- Automated tests (recommended): add unit tests around db helpers and inventory logic (Catch2/GoogleTest).

## Contribution

The participation of everyone is needed to make this project a success. Please follow the guidelines below when contributing to this project.

### Naming Conventions
- Functions
  - lowerCamelCase (e.g., createDatabase, switchToUI).
- Local variables / parameters: 
  - lowerCamelCase (e.g., fontPath, createTableSql).
  - variable kind(global, constant, local, ) then underscore, then the variable name (e.g., g_windowWidth).
- Types and classes
  - PascalCase.
- File names:
  - lowercase_with_underscores.cpp / .h.
- UI registry keys
  - lowercased strings (use toLower helper before lookup).

### Coding Style
- Language level: C++17.
- Resource management: prefer RAII; always close sqlite handles and free error strings.
- Passing arguments: use const T& for non-trivial types (std::string const&).
- Return pattern for C-style APIs: return bool for success and use an optional std::string* outError for human-readable errors (follow existing db.* style).
- Error reporting: prefer sqlite3_errmsg(handle) when handle exists, otherwise sqlite3_errstr(code).
- Avoid duplicating open/exec/close logic — use a small private helper (execSql) for one-off SQL work.
- UI code: keep presentation-only. No DB or business logic in UI functions; call domain/db APIs from thin adapters.
- Fonts: attempt to load font then fall back to AddFontDefault(); call io.Fonts->Build() when modifying fonts.
- Use std::clamp and input sanitization for window sizes; reserve container capacities where repeated insertions are expected (e.g., g_uiMap.reserve()).
- Use std::boolalpha when printing booleans for readable logs.

### Module Responsibilities and Organization
- main.cpp
  - Responsibilities: program entry, basic diagnostics (e.g., db::isSqliteAvailable), create/open DB, configure and start the UI runner.
  - Keep minimal: no business logic, no direct DB schema work (call db helpers instead).
- src/lib/UI.cpp
  - Responsibilities: immediate-mode UI only — register UIs, load fonts, set HelloImGui params, invoke g_currentUI each frame.
  - Conventions: register UI handlers once, use lowercase keys, perform case-insensitive switching, keep each UI function small and focused, avoid long inline logic, and ensure ShowGui is a simple wrapper that calls the current UI.
- src/lib/db.cpp / db.h
  - Responsibilities: thin, well-documented wrappers around sqlite3: availability checks, create/open DB, execute statements, schema helpers.
  - Conventions: each public helper must open the DB with sqlite3_open_v2 (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), run the operation, set outError on failure, and close the handle before returning. Document arguments and possible outputs in a short C-style or Doxygen comment above the function.

### Commit Messages
- Format: "<area>: <imperative summary>"
  - Examples: "db: add createTable helper", "ui: move font loading to LoadAdditionalFonts".
- Include a brief body when needed: one-line rationale and test instructions.
- Reference issue/PR numbers when applicable.

### Quick examples (follow these patterns)
- DB call pattern:
```cpp
std::string err;
if (!db::createDatabase("app.db", &err)) {
    // handle error; err contains human-readable message
}
```
- UI registration:
```cpp
g_uiMap.reserve(4);
g_uiMap["main"] = mainUI; // keys stored lower-case
```
