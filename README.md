<p align="center">icon</p>
<h1 align="center">Project Title</h1>

<h6 align="center">This repository is the final project for CpET 140 - Computer Programming 1 </h6>

<h3 align="center">Members</h3>

<div align="center">
    <table style="width:50%; margin:0 auto;">
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

### Project Highlights
- Language: C++17 (or later)
- Build system: CMake
- UI: Hello ImGui (immediate-mode GUI)
- Database: SQLite (single-file, local, embedded)
- Target platforms: Desktop (Win/macOS/Linux), Mobile (Android/iOS)

### Repository Structure
  ```
    cpet-140-final-project/
    ├─ assets/
    │  └─ fonts/
    │     └─ OpenSans-Regular.ttf
    ├─ dependencies/
    │   └─ sqlite3/
    │      ├─ sqlite3.c
    │      └─ sqlite3.h
    ├─ doc/
    │  ├─ CpET 140 - Computer Programming 1 - Final Output.pdf
    │  └─ hello_imgui_manual.pdf
    ├─ src/
    │  ├─ main.cpp
    │  └─ lib/
    │     ├─ UI.cpp
    │     ├─ db.cpp
    │     ├─ db.h
    │     ├─ inventory.cpp
    │     └─ pos.cpp
    ├─ .gitignore
    ├─ CMakeLists.txt
    └─ README.md
  ```

## Application Structure
- Modules communicate via simple C++ interfaces; DB functions open/close connections per operation unless a shared handle is used.


## Build & Run (desktop)

### Prerequisites
- C++26
- CMake 4.0 or later
- sqlite3 3.50.4
- Hello ImGui

### Common commands (from repo root)
- Configure and build:

        mkdir -p build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release
        cmake --build . --config Release

- Run:

      ./your-executable-name

### Mobile
- Android: there is a separate repository for the Android version with appropriate Gradle/CMake setup; run via Android Studio or command line with SDK/NDK.
- iOS: A separate repository contains an Xcode project; open in Xcode and run on simulator/device.


## Dependencies

- sqlite3 (runtime + development headers)
- ImGui (library or submodule; integrate with your renderer)
- Optional:
  - libraries for platform windows/rendering (GLFW/SDL/DirectX/Metal)


## Testing & Validation

- Manual smoke tests: 
  - startup
  - DB create/open
  - add item
  - create sale
  - close and reopen DB
  - UI interactions.
- Automated tests (recommended): add unit tests around db helpers and inventory logic (Catch2/GoogleTest).


## Contribution

The participation of everyone is needed to make this project a success. Please follow the guidelines below when contributing to this project.

### Naming Conventions
- Functions / Methods:
  - lowerCamelCase (e.g., createDatabase, switchToUI).
  - Prefixes: verb indicating action (get, set, create, load, save etc.).
  - Suffixes: indicate type of operation (e.g., Async for asynchronous operations).


- Types and classes:
  - lowerCamelCase (e.g., dbAccess)


- Local variables / parameters / constants: 
  - lowerCamelCase (e.g., fontPath).
  - Prefix:
    - g_ for globals (e.g., g_currentUI)
    - c_ for constants (e.g., c_defaultFontSize)
    - l_ for locals (e.g., l_itemCount).
  - Suffix: any verb, adjective or noun indicating purpose (e.g., count, size, index).
    - for variables representing a unit of something, use the unit as suffix (e.g., priceUsd, sizePx).

### Coding Style
- Language level: C++17.

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