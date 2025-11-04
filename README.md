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
- there is a separate repository for the Android version with appropriate setup and build instructions for both Android and iOS.


## Dependencies

- sqlite3 (runtime + development headers)
- Hello ImGui 
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
  - lowerCamelCase (e.g., `createDatabase`, `switchToUI`).
  - Prefixes: verb indicating action (get, set, create, load, save, open, close etc.).
  - Suffixes: use clear, descriptive suffixes that indicate the operation's target or context (for example, use `Database` for functions that perform database operations).
  - Exemptions: `main()` function.
  - Examples:
    - `createDatabase()`
    - `constructUI()`


- Types and classes:
  - lowerCamelCase (e.g., `dbAccess`)


- Local variables / parameters / constants: 
  - lowerCamelCase (e.g., `fontPath`).
  - Prefix:
    - g_ for globals (e.g., `g_currentUI`)
    - c_ for constants (e.g., `c_defaultFontSize`)
    - l_ for locals (e.g., `l_itemCount`).
  - Suffix: any verb, adjective or noun indicating purpose (e.g., count, size, index).
    - for variables representing a unit of something, use the unit as suffix (e.g., priceUsd, sizePx).


### Module Responsibilities and Organization
- main.cpp
  - Responsibilities: program entry, basic diagnostics and configure and start the UI runner.
  - Conventions: no logic, no direct DB schema work


- src/lib/UI.cpp and UI.h
  - Responsibilities: immediate-mode UI only — register UIs, load fonts, set HelloImGui params, invoke g_currentUI each frame.
  - Conventions: register UI handlers, keep UIs small, avoid long inline logic, ShowGui should simply call the current UI.

- src/lib/inventory.cpp and inventory.h
  - Responsibilities: inventory item, list/filter items, stock adjustments.
  - Conventions: keep inventory logic separate from DB, validate inputs, return clear error codes.


- src/lib/pos.cpp and pos.h
  - Responsibilities: sales transactions, cart management, payment processing.
  - Conventions: separate logic from DB, handle payment calculations, ensure data integrity.


- src/lib/db.cpp and db.h
  - Responsibilities: availability checks, create/open/modify DB, execute statements, schema helpers.
  - Conventions: keep DB helpers small, use RAII for connections, prefer prepared statements, and always check SQLite return codes.

### Commit Messages
- Format: "<area>: <imperative summary>"
  - Examples: "db: add createTable helper", "ui: move font loading to LoadAdditionalFonts".
- Include a brief body when needed: one-line rationale and test instructions.
- Reference issue/PR numbers when applicable.