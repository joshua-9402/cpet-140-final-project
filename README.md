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
- Language: C++26
- Build system: CMake v4.0
- UI: Hello ImGui (immediate-mode GUI) v1.19.3
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
    │     ├─ UI.h
    │     ├─ db.cpp
    │     ├─ db.h
    │     ├─ inventory.cpp
    │     ├─ inventory.h
    │     ├─ pos.cpp
    │     └─ pos.h
    ├─ .gitignore
    ├─ CMakeLists.txt
    └─ README.md
  ```

## Application Structure

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

### API / Interfaces
- From `main.cpp`
    - `main()` function
        - starts the whole application
        - checking for dependencies and libraries
        - constructs UI
- From `UI.cpp`
    - `constructUI()` function
        - sets up Hello ImGui
        - loads fonts
        - registers UIs
    - `createFailedMessage()` function
        - the `g_failedMessage` variable is not accessible in other modules, so this function is used to create a failed message needed.
- From `db.cpp`
    - `createDatabase()` function
        - creates a new SQLite database file with the required schema
    - `openDatabase()` function
        - opens an existing SQLite database file
    - `executeSQL()` function
        - executes a given SQL statement with optional parameters


## Build & Run (Desktop)

### Prerequisites
- C++26 or later (the compiler must support C++26)
- CMake 4.0 or later
- sqlite3 3.50.4
- Hello ImGui

### Commands (building from repository's root)
- Configure and build:

        mkdir -p build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Release
        cmake --build . --config Release

- Run:

      ./your-executable-name


## Build and Run (Mobile)

- there is a separate <a href="https://github.com/joshua-9402"> repository </a> for the Android and iOS version with appropriate setup and build instructions for both Android and iOS.


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


## Contribution Guidelines
The participation of everyone is needed to make this project a success. Please follow the guidelines below when contributing to this project.

> Notes:
> - `UI.cpp`, `UI.h`, `db.cpp` and `db.h` are OFF LIMITS especially for `constructUI()` and `switchToUI()` functions (in `UI.cpp`).
>   - For bugs, please file an issue instead.
>   - For enhancements, please discuss with the maintainers first or file an issue regarding the enhancement.
>   - For adding new UIs, please file an issue first to discuss the addition.
> - When contributing code, please ensure that your code adheres to the coding conventions outlined below.
> - Any significant deviations from the coding conventions should be explained in the PR / commit description, or it will be rejected.

### Naming Conventions
- Types and Classes:
    - lowerCamelCase (e.g., `dbAccess`)


- Functions / Methods:
  - lowerCamelCase (e.g., `createDatabase`, `switchToUI`).
  - Prefixes: verb indicating action (get, set, create, load, save, open, close etc.).
  - Suffixes: use clear, descriptive suffixes that indicate the operation's target or context (for example, use `Database` for functions that perform database operations).
  - Exemptions: `main()` function.
  - Examples:
    - `createDatabase()`
    - `constructUI()`


- Local variables / parameters / constants: 
  - lowerCamelCase (e.g., `fontPath`).
  - Prefix (only parameters are exempted):
    - `g_` for globals (e.g., `g_currentUI`)
    - `c_` for constants (e.g., `c_defaultFontSize`)
    - `l_` for locals (e.g., `l_itemCount`).
  - Suffix: any verb, adjective or noun indicating purpose (e.g., count, size, index).
    - for variables representing a unit of something, use the unit as suffix (e.g., `priceUsd`, `sizePx`).

### Commit Messages
- Format: "<action>: <description>"
  - For actions, use a verb indicating the change
    - `add` - addition of new functionality / file / module / feature
    - `remove` - deletion of functionality / file / module / feature
    - `fix` - bug fix
    - `update` - update existing functionality / dependencies / docs
    - `improve` - enhancement of existing functionality / performance / UX
    - `refactor` - code restructuring without changing behavior
    - `chore` - maintenance tasks (build scripts, CI config, etc.)
    - `build` - changes to build process or dependencies
      - this is different from `chore` since it directly affects the build process or dependencies
      - this option is only available for `CMakeLists.txt` or build scripts
    - the only exception is `initial commit` and the few subsequent commits in the repository and the `README.md`.
      - `README.md` follows a different convention since it is a documentation file.
        - `docs` - update README.md, add technical overview, fix typos, etc.
  - For description, use a concise summary of the change.
  - Examples: `db: add createTable helper`, `ui: move font loading to LoadAdditionalFonts`.
- Include a brief body when needed: one-line rationale and test instructions.
- Reference issue/PR numbers when applicable.
- Examples: 
  - "fix: correct inventory item deletion logic (#42)"
  - "update: upgrade sqlite3 to 3.50.4 for security patches (#56)"

### Filing Issues
- The use of Filipino language is allowed when filing issues.
- Use clear, descriptive titles.
- Provide detailed descriptions, including steps to reproduce, expected vs. actual behavior, and screenshot or the error logs.
- Assign appropriate labels:
  - bug - something is not working as expected
  - documentation - improvements or additions to documentation
  - enhancement - new feature or request
  - help wanted - assistance needed
  - question - request for information or clarification
- Reference related issues or PRs when applicable.


### Filing Pull Requests
- Ensure your branch is up-to-date with the main branch before creating a PR.
- Provide a clear title and description of the changes made.
- Reference any related issues.
- Follow the project's coding conventions and guidelines. Any significant deviations should be explained in the PR description, or it will be rejected.
- Ensure all tests pass before submitting the PR. Submitted PRs will undergo for a review and also include the test results.

### Troubleshooting & Debugging Tips
- For `main.cpp` module
  - Make sure that all functions / methods are correctly called
  - Only put valid values when calling functions / methods or configuring application variables to avoid unexpected behaviors.
  - For `main()` function:
    - Ensure that all dependencies are correctly initialized before starting the UI.
    - For `constructUI()`, make sure to correctly match the variables and the arguments.


- For `UI.cpp` module
  - The `switchUI()`, `mainUI()`, `posUI()`, `inventoryUI()`, and `failedUI()` are private functions. No one except the functions / methods  in the `UI.cpp` can access it.
  - For `constructUI()`, this function is STRICTLY OFF-LIMITS as it contains critical UI initialization logic; do not modify it without prior approval from the project maintainers.
  - For `createFailedMessage()`, this function is used to create a failed message when the application fails to start. Do not modify it without prior approval from the project maintainers.


- For `pos.cpp`


- For `inventory.cpp`


- For `db.cpp` and database-related module / dependencies
  - This module is STRICTLY OFF-LIMITS.
  - Enable verbose SQLite logging during development (sqlite3_log).
  - Add a debug mode to log SQL statements and durations.
  - Reproduce issues with a temporary DB and add unit/regression tests once fixed.
  - Calling methods from `db.cpp` is `<db>::method` (e.g., `db::createDatabase(<path>, <error>)`)
  - For `isSqliteAvailable()` method
    - Returns either `true` or `false with error code`.
  - For `createDatabase()`
    - It will create a new database or open a database (if there is an existing database)
    - Returns either `true` or `false with error`

### Configuration
- Configuration is intentionally minimal and file-based for portability.
- Load configuration early in main.cpp and allow environment variables to override file values for CI/containers.

### Known Limitations
- Single-file SQLite limits multi-user concurrent writes; WAL helps but not a replacement for a server DB in multi-client scenarios.
- No built-in encryption of the DB file; consider SQLCipher or OS-level disk encryption for sensitive deployments.
- UI is immediate-mode and desktop-focused; mobile UI experience requires a separate UI layer implementation.