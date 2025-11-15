<p align="center">icon</p>
<h1 align="center">StructuraCost</h1>

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
        <td> <a href="https://github.com/mattguina-cyber"> mattguina-cyber </a> </td>
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
        <tr>
            <td>TENORIO, HERSHEY MAE D.</td>
            <td></td>
    </table>
</div>


---

## Table of Contents

I. Documentation
  - [Technical Overview](#technical-overview)
    - [Project Highlights](#project-highlights)
    - [Repository Structure](#repository-structure)
  - [Application Structure](#application-structure)
    - [Module Responsibilities](#module-responsibilities)
    - [Interfaces](#interfaces)
    - [Calling / Invoking Conventions](#calling--invoking-conventions)

II. Getting Started
  - [Automated Builds (CI/CD)](#automated-builds-cicd)
  - [Build & Run (Desktop)](#build--run-desktop)
    - [Prerequisites (For Development Computer Only)](#prerequisites-for-development-computer-only)
    - [Commands (building from repository's root)](#commands-building-from-repositorys-root)
  - [Build and Run (Mobile)](#build-and-run-mobile)
  - [Dependencies](#dependencies)

III. Quality & Contributions
  - [Testing & Validation](#testing--validation)
  - [Contribution Guidelines](#contribution-guidelines)
    - [Whitespace and Formatting](#whitespace-and-formatting)
    - [Naming Conventions](#naming-conventions)
    - [Commit Messages](#commit-messages)
    - [Filing Issues](#filing-issues)
    - [Filing Pull Requests](#filing-pull-requests)
    - [Troubleshooting & Debugging Tips](#troubleshooting--debugging-tips)

---


## Technical Overview

This project is a C++ application that demonstrates a payroll and monitoring system with a UI layer and SQLite-backed persistence. It is designed for desktop platforms (Windows, macOS, Linux) and can be adapted to mobile platforms (Android, iOS) with platform-specific toolchains.

### Project Highlights
- Language: C++23
- Build system: CMake v4.0
- UI: Hello ImGui (immediate-mode GUI) v1.19.3
- Database: SQLite (single-file, local, embedded)
- Target platforms: Desktop (Win/macOS/Linux), Mobile (Android/iOS)

### Repository Structure
      cpet-140-final-project/
      ├── assets/
      │   ├── fonts/
      │   │   └── OpenSans-Regular.ttf
      │   └── icons/
      │       ├── business_logo.png
      │       └── user_icon.png
      ├── assets (raw)/
      │   └── concept.png
      ├── dependencies/
      │   └── sqlite3/
      │      ├── sqlite3.c
      │      └── sqlite3.h
      ├── doc/
      │   ├── system_flowcart/
      │   │   └── CpET 140 Final Project System Flowchart.svg
      │   ├── CpET 140 - Computer Programming 1 - Final Output.pdf
      │   └── hello_imgui_manual.pdf
      ├── src/
      │   ├── main.cpp
      │   ├── config/
      │   │   ├── app_config.cpp
      │   │   └── app_config.h
      │   ├── handler/
      │   │   ├── db.cpp
      │   │   ├── db.h
      │   │   ├── io.cpp
      │   │   ├── io.h
      │   │   ├── system.cpp
      │   │   └── system.h
      │   ├── core/
      │   │   ├── monitor.cpp
      │   │   ├── monitor.h
      │   │   ├── payroll.cpp
      │   │   └── payroll.h
      │   ├── security/
      │   │   ├── auth.cpp
      │   │   └── auth.h
      │   └── ui/
      │       ├── ui.cpp
      │       └── ui.h
      ├── .gitignore
      ├── CMakeLists.txt
      └── README.md


## Application Structure

### Module Responsibilities
- `main.cpp`
    - Responsibilities: program entry, basic diagnostics and configure and start the UI runner.
    - Conventions: no logic, no direct DB schema work


- `src/config/app_config.cpp` and `app_config.h`
    - Responsibilities: load/save app configuration (file paths, UI settings).
    - Conventions: simple getters/setters, no UI or DB logic.


- `src/ui/UI.cpp` and `UI.h`
    - Responsibilities: immediate-mode UI only — register UIs, load fonts, set HelloImGui params, invoke g_currentUI each frame.
    - Conventions: register UI handlers, keep UIs small, avoid long inline logic, ShowGui should simply call the current UI.


- `src/handler/db.cpp` and `db.h`
    - Responsibilities: availability checks, create/open/modify DB, execute statements, schema helpers.
    - Conventions: keep DB helpers small, use RAII for connections, prefer prepared statements, and always check SQLite return codes.


- `src/handler/auth.cpp` and `auth.h`
    - Responsibilities: user authentication and authorization (login, logout, session management).
    - Conventions: no UI or DB schema work — use `db::` helpers; prefer secure handling of credentials; small, testable lowerCamelCase functions; return explicit result types and add unit tests; avoid global state.


- `src/handler/system.cpp` and `system.h`
    - Responsibilities: system-level operations (file I/O, environment checks, logging).
    - Conventions: no UI or DB schema work — use `db::` helpers; small, testable lowerCamelCase functions; return explicit result types and add unit tests; avoid global state.


- `src/core/payroll.cpp` and `src/lib/payroll.h`
  - Responsibilities: payroll logic (records, rates, time entries, deductions, taxes), CRUD (Create, Read, Update, and Delete) and payroll runs; validate inputs and produce deterministic calculations. Delegate persistence to `db`.
  - Conventions: no UI or raw SQLite calls — use `db::` helpers; prefer RAII and integer/fixed\-point for money; small, testable lowerCamelCase functions; return explicit result types and add unit tests; avoid global state.


- `src/core/monitor.cpp` and `monitor.h`
  - Responsibilities: collect and expose domain-specific expense metrics (per-project expense totals, payroll/outflow summaries, invoice/payment status); aggregate and persist expense snapshots via `db::` helpers on demand; provide a synchronous, lightweight query API for the UI and `payroll` module; validate inputs and emit alerts/logs for threshold breaches.
  - Conventions: no system monitoring (CPU/memory) and no background threads/polling/timers here; delegate persistence to `db::` helpers

### Interfaces
- From `main.cpp`
    - `main()` function
        - starts the whole application
        - checking for dependencies and libraries
        - constructs UI

- From `app_config.cpp`
    - `g_appTitle;`
    - `g_loginTitle;`
    - `g_fontName;`
    - `g_dbNamePayroll;`
    - `g_dbNameTracker;`
    - `g_txtNameUser;`
    - `g_defaultWidth;`
    - `g_defaultHeight;`
    - `g_smallWidth;`
    - `g_smallHeight;`

- From `UI.cpp`
    - `constructUI()` function
        - sets up Hello ImGui
        - loads fonts
        - registers UIs

- From `db.cpp`
    - `createFileText()` function
        - creates a text file at the specified path with the given content
    - `readFileText()` function
        - reads the content of a text file at the specified path
    - `appendFileText()` function
        - appends content to a text file at the specified path
    - `createDatabase()` function
        - creates a new database
    - `openDatabase()` function
        - opens an existing database
    - `closeDatabase()` function
        - closes the database connection
    - `appendToDatabase()` function
        - appends data to the database

- From `system.cpp`
    - `fetchTime()` function
        - fetches the current system time
    - `createDirectory()` function
        - creates a new directory at the specified path

- From `auth.cpp`
    - `authGateway()` function
        - handles user authentication and authorization

### Calling / Invoking Conventions
This section documents how modules in the repository should be invoked, the minimal contracts (inputs/outputs), common error modes, and a few examples. The conventions are based on the current repository layout and naming conventions (globals start with `g`, local variables with `l`, etc.).

1) General rules
   - Prefer calling the well-scoped API functions from headers (e.g., `db::`, `system::`, `auth::`). Do not reach into internal implementation files unless there is no public API.
   - Globals: read-only access is acceptable; modifications should be centralized (for example, set `g_currentUI` only via a UI switch function).
   - Side effects should be explicit in the function name (e.g., `createDatabase()` creates/persists a database file; `appendFileText()` mutates a file).

2) Methods, functions or global variable/s (with conventions)
   - for `db::` functions (in `db.cpp`)
     - `db::createDatabase(const std::string &p_dbName)`
       - Inputs: `p_dbName` = path to DB file
       - Outputs: returns boolean value (`true` on success, `false` on failure)
       - Errors: file system permission errors, sqlite open/locking errors
       - Side effects: creates the file and directories as needed

     - `db::openDatabase(const std::string &p_dbName)`
       - Inputs: `p_dbName` = path to DB file
       - Outputs: returns boolean value (`true` on success, `false` on failure)
       - Errors: missing file, incompatible schema, sqlite errors
       - Caller responsibility: call `db::closeDatabase()` on the handle when done
       
     - `db::closeDatabase(const std::string& p_dbName)`
       - Inputs: `p_dbName` = path to DB file
       - Outputs: returns boolean value (`true` on success, `false` on failure)
       - Errors: sqlite errors during close
       - Side effects: releases DB handle/resources

     - `db::appendToDatabase(const std::string& p_dbName, std::string& p_data)`
       - Inputs: `p_dbName` = path to DB file, `p_data` = data to append
       - Outputs: returns boolean value (`true` on success, `false` on failure)
       - Errors: SQL syntax, constraint violations; always inspect return

   - for `system::` functions (in `system.cpp`)
     - `system::fetchTime(const PartDateTime part)`
       - Inputs: `part` = enum selecting portion to return (examples: `Full`, `Date`, `Time`, `Year`, `Month`, `Day`)
       - Outputs: returns `int` with the requested date or time part
       - Errors: system clock access failure, invalid `part` value
       - Note: keep function side\-effect free; callers handle timezone/locale conversions
       
     - `system::createDirectory(const std::string &directoryName)`
       - Inputs: directory path
       - Outputs: returns boolean value (`true` on success, `false` on failure)
       - Errors: permission denied, invalid path, already exists
       - Note: callers should check for existence before creating (to avoid errors)
       
   - for `auth::` functions (in `auth.cpp`)
     - `auth::authGateway(const std::string &username, const std::string &password, const std::string &source)`
       - Inputs: credentials and optional source tag
       - Outputs: returns an auth result (bool or enum) and sets session/globals
       - Errors: invalid credentials, DB errors
       - Side effects: may set `g_auth` or a session token; prefer returning an explicit result instead of relying on globals

   - for `ui::` functions (in `ui.cpp`)
     - `constructUI(const std::string &a_title, const std::string& a_fontLocation, const int a_widthPx, const int a_heightPx, const std::string& a_window)`
       - Inputs: title, font path, window size (width and height), assets folder
       - Outputs: registers UI handlers with HelloImGui and loads fonts/resources
       - Errors: missing assets, font load errors — log and fall back to defaults
     - `g_errorMessage` — global string to hold error messages for UI display

3) Example call flows
   - Startup (in `main.cpp`):
     - call config load functions to populate `g_*` variables
     - call `system::createDirectory()` for app data directory if needed
     - call `db::createDatabase()` or `db::openDatabase()` to ensure persistence is available
     - call `constructUI()` to initialize the UI system
     - start the HelloImGui runner

   - Login flow (high-level):
     - Login UI calls `auth::authGateway(username, password, "loginUI")`
     - On success: set `g_auth = true` (or return an auth token). `main.cpp` observes `g_auth` and calls `switchToUI(MainUI)`
     - On logout: call `auth::logout()` (if present) which clears session and sets `g_auth = false`; then call `switchToUI(LoginUI)`

4) Error and edge-case handling (recommended patterns)
   - Always check and propagate error messages (use `outError` strings) rather than silently ignoring failures.
   - For filesystem operations, handle the "already exists" case as success (unless replacing is intended).
   - For DB operations: prefer prepared statements and validate inputs before executing.
   - For UI asset loads: call `HelloImGui::SetAssetsFolder()` at startup to avoid brittle relative-path lookups.

5) Minimal examples
   - Call `system::createDirectory()` safely:

           // pseudo-call (conceptual)
           std::string path = "/path/to/appdata";
           if (!system::createDirectory(path)) {
               // handle failure: log and show a friendly UI error
           }

   - Login (conceptual):

           // in login UI handler
           std::string err;
           auto ok = auth::authGateway(username, password, "loginUI");
           if (ok) {
               // ask main loop to switch UI (set flag or call switchToUI)
           } else {
               // show error
           }

6) Notes about globals used in the codebase
   - `g_currentUI` — read-only from most files; only `ui.cpp` (or a dedicated switch function) should change it.
   - `g_auth` — represents authentication state. Prefer exposing a small API (e.g., `auth::isAuthenticated()`) rather than reading the global directly.
   - `g_smallWidth`, `g_smallHeight`, `g_defaultWidth`, `g_defaultHeight` — layout hints for UIs. UIs should use these as suggestions, not absolute constraints.

7) Quick checklist for contributors when calling modules
   - Read the header for the module you call and prefer the public API.
   - Check return codes and propagate errors to the caller/UI.
   - Avoid changing globals directly; use switch/setter functions where present.
     - Keep UI code free of raw DB operations; call `db::` helpers instead.


## Automated Builds (CI/CD)

This project includes both automated and manual builds via GitHub Actions.

### Manual Build Workflows (macOS)

For macOS, we provide **three manual-trigger workflows** to support different hardware and OS versions:

| Workflow                 | Target Hardware          | macOS Versions                                  | C++ Std  | Use Case                  |
|--------------------------|--------------------------|-------------------------------------------------|----------|---------------------------|
| **Build macOS ARM64**    | Apple Silicon (M1/M2/M3) | Sonoma 14.0+                                    | C++23    | Modern Apple Silicon Macs |
| **Build macOS Intel**    | Modern Intel Macs        | Big Sur 11.0+                                   | C++23    | Modern Intel Macs         |
| **Build macOS Legacy**   | Older Intel Macs         | Catalina 10.15, Mojave 10.14, High Sierra 10.13 | C++20    | Legacy Intel Macs         |

**To trigger:** Go to Actions → Select workflow → Run workflow

See `.github/workflows/README-*.md` for detailed instructions on each workflow.

### Downloading Pre-built Binaries

#### From GitHub Actions (Automated Builds)
1. Navigate to the **Actions** tab in the repository
2. Click on the latest successful workflow run
3. Scroll to the **Artifacts** section
4. Download the artifact for your platform:
   - `structuracost-linux-x64.tar.gz`
   - `structuracost-windows-x64.zip`

#### From Manual macOS Builds
1. Navigate to the **Actions** tab
2. Select the appropriate macOS workflow (ARM64, Intel, or Legacy)
3. Click **Run workflow** and configure your build
4. Wait for the build to complete
5. Download the artifact from the workflow run page

#### From Releases (Tagged Versions)
When a version tag is created (e.g., `v1.0.0`), platform builds may be automatically attached to a GitHub Release.


For detailed CI/CD documentation, see [.github/BUILD.md](.github/BUILD.md).


## Build & Run (Desktop)

### Prerequisites (For Development Computer Only)
- C++20 or later
> - It should be at C++26 but Apple Clang partially support C++26 
- CMake 3.22 or later
- sqlite3 3.50.4
- Hello ImGui

### Commands (building from repository's root)
- Configure, build and run in RELEASE:

        cmake -S . -B build_release -DCMAKE_BUILD_TYPE=Release
        cmake --build build_release
        open build_release/structuracost.app


## Build and Run (Mobile)

- There is a separate <a href="https://github.com/joshua-9402/cpet-140-final-project-mobile"> repository </a> for the Android and iOS version with appropriate setup and build instructions for both Android and iOS.


## Dependencies

- sqlite3 (runtime + development headers)
- argon2 (for password hashing, runtime + development headers)
- Hello ImGui 
- Optional:
  - libraries for platform windows/rendering (GLFW/SDL/DirectX/Metal)


## Testing & Validation

- Manual smoke tests: 
  - startup
  - DB create/open
  - add/edit/delete inventory items
  - add/edit/delete payroll transactions
  - close and reopen DB
  - UI interactions.
- Automated tests (recommended): add unit tests around db helpers and inventory logic (Catch2/GoogleTest).


## Contribution Guidelines
The participation of everyone is needed to make this project a success. Please follow the guidelines below when contributing to this project.

> Notes:
> - `UI.cpp`, `UI.h`, `db.cpp` and `db.h` are OFF LIMITS especially for `constructUI()` and `switchToUI()` functions (in `UI.cpp`).
>   - For bugs, please file an issue instead.
>   - For enhancements, please discuss with the maintainer first or file an issue regarding the enhancement.
>   - For adding new UIs, please file an issue first to discuss the addition.
> - When contributing code, please ensure that your code adheres to the coding conventions outlined below.
> - Any significant deviations from the coding conventions should be explained in the PR / commit description, or it will be rejected.
> - The use of other languages other than English is not allowed unless specified.

### Whitespace and Formatting
- Use 4 spaces or 1 tab for indentation .
- Limit lines to a maximum of 20 words (comments only) for better readability.
  - This restriction applies only to inline comments within source files; header comments, top-of-function/method comments, and documentation files are exempted.
- Use blank lines to separate logical sections of code.
  - For spaces in function / methods:
    - give two line for each function / method definition and implementation.

### Naming Conventions
- Types and Classes:
    - lowerCamelCase (e.g., `dbAccess`)


- Functions / Methods:
  - lowerCamelCase (e.g., `createDatabase`, `switchToUI`).
  - Prefixes: verb indicating action
    - Example:
      - `get` - retrieve a value or resource from a known location (e.g., read a variable, fetch a record or file)
      - `set` - assign or update a value or configuration (e.g., update a variable, modify a record or file)
      - `create` - allocate or persist a new resource or object 
      - `load` - load a file or resource into application memory
      - `save` - persist in-memory data to storage
      - `delete` - remove or deallocate a resource or object
      - `update` - modify an existing resource or object
      - `open` - open a resource or connection for use
      - `close` - release or shut down a resource or connection
      - `convert` - transform data between formats or units
      - `change` - modify state or configuration
  - Suffixes: use clear, descriptive suffixes that indicate the operation's target or context (for example, use `Database` for functions that perform database operations).
  - Exemptions: 
    - The `main()` function in `main.cpp`.
    - The UI handler functions in `UI.cpp` (e.g., `mainUI()`, `payrollUI()`, `monitorUI()`, `failedUI()`).
    - Examples:
      - `createDatabase()`
      - `constructUI()`


- Variables (Local, Global, and Constant) and parameters:
  - lowerCamelCase (e.g., `fontPath`).
  - Prefix:
    - `g` for global variable (e.g., `g_currentUI`)
    - `l` for local variable (e.g., `l_itemCount`).
    - `c` for constant variable (e.g., `c_defaultFontSize`)
    - `p` for parameter (e.g., `p_machineCode`)
  - Suffix: any verb, adjective or noun indicating purpose (e.g., count, size, index).
    - for variables representing a unit of something, use the unit as suffix (e.g., `c_priceUsd`, `g_sizePx`).

### Commit Messages
- Format: "<action>: <description>"
  - For actions, use a verb indicating the change
    - `add` - addition of new functionality
    - `feat` - addition of a significant new feature
    - `remove` - deletion of functionality / file / module / feature
    - `fix` - bug fix
    - `update` - update existing functionality / dependencies / docs
    - `improve` - enhancement of existing functionality / performance / UX
    - `refactor` - code restructuring without changing behavior
    - `chore` - maintenance tasks (build scripts, CI config, etc.)
    - `build` - changes to build process or dependencies
      - this is different from `chore` since it directly affects the build process or dependencies
      - this option is only available for `CMakeLists.txt` or build scripts
    - the only exception is `initial commit` and the few subsequent commits in the repository and `README.md`
      - `README.md` follows a different convention since it is a documentation file.
        - `docs` - update README.md, add technical overview, fix typos, etc.
  - For description, use a concise summary of the change.
  - Examples: `db: add createTable helper`, `ui: move font loading to LoadAdditionalFonts`.
- Include a brief body when needed: one-line rationale and test instructions.
- Reference issue/PR numbers when applicable.
- Examples: 
  - `fix: correct inventory item deletion logic (#42)`
  - `update: upgrade sqlite3 to 3.50.4 for security patches (#56)`

### Filing Issues
- The use of Filipino language is allowed when filing issues.
- Use clear, descriptive titles.
- Provide detailed descriptions, including steps to reproduce, expected vs. actual behavior, and screenshot or the error logs.
- Assign appropriate labels:
  - `bug` - something is not working as expected
  - `documentation` - improvements or additions to documentation
  - `enhancement` - new feature or request
  - `help wanted` - assistance needed
  - `question` - request for information or clarification
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


- For `payroll.cpp`


- For `monitoring.cpp`


- For `db.cpp` and database-related module / dependencies
  - This module is STRICTLY OFF-LIMITS.
  - Enable verbose SQLite logging during development (sqlite3_log).
  - Add a debug mode to log SQL statements and durations.
  - Reproduce issues with a temporary DB and add unit/regression tests once fixed.
  - Calling methods from `db.cpp` is `<db>::method` (e.g., `db::createDatabase(<path>, <error>)`)
  - For `createDatabase()`
    - It will create a new database or open a database (if there is an existing database)
    - Returns either `true` or `false with error`
