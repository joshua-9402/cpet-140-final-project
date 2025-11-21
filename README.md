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
  - [Build & Run (Desktop)](#build--run-desktop)
    - [Prerequisites (For Development Computer Only)](#prerequisites-for-development-computer-only)
    - [Automated Builds (CI/CD)](#automated-builds-cicd)
      - [Manual Build](#manual-build)
      - [Windows Builds](#windows-builds)
      - [macOS Builds](#macos-builds)
      - [Linux Builds](#linux-builds)
      - [Release Builds (All Platforms)](#release-builds-all-platforms)
      - [Downloading Pre-built Binaries](#downloading-pre-built-binaries)
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

This project is a C++ application that demonstrates a payroll and monitoring system with a UI layer and SQLite-backed persistence. It is designed for desktop platforms (Windows, macOS, Linux).

### Project Highlights
- Language: C++20
- Build system: CMake v3.22
- UI: Hello ImGui (immediate-mode GUI) v1.19.3
- Database: SQLite (single-file, local, embedded)
- Cryptography: libsodium (for password hashing, encryption, decryption, salting, etc.)
- Architecture: modular with clear separation of concerns (UI, DB, core logic, system)
- Target platforms: Desktop (Win/macOS/Linux), Mobile (Android/iOS)

### Repository Structure
      cpet-140-final-project/
      ├── .github/
      │   └── workflows/
      │       ├── BULD.md
      │       ├── README-linux.md
      │       ├── README-linux-rpm.md
      │       ├── README-linux-suse.md
      │       ├── README-macos-arm64.md
      │       ├── README-macos-intel.md
      │       ├── README-windows-arm64.md
      │       ├── README-windows-x86_64.md
      │       ├── LIBSODIUM_CI_SUMMARY.md
      │       ├── build-linux.yml
      │       ├── build-linux-rpm.yml
      │       ├── build-linux-suse.yml
      │       ├── build-macos.yml
      │       ├── build-windows.yml
      │       └── release.yml
      ├── assets/
      │   ├── fonts/
      │   │   └── OpenSans-Regular.ttf
      │   └── icons/
      │       ├── business_logo.png
      │       └── user_icon.png
      ├── assets (raw)/
      │   ├── user_icon.psd
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
      │   │   ├── config.cpp
      │   │   └── config.h
      │   ├── handler/
      │   │   ├── db.cpp
      │   │   ├── db.h
      │   │   ├── system.cpp
      │   │   └── system.h
      │   ├── core/
      │   │   ├── monitor.cpp
      │   │   ├── monitor.h
      │   │   ├── payroll.cpp
      │   │   └── payroll.h
      │   ├── security/
      │   │   ├── cryptography.cpp
      │   │   ├── cryptography.cpp
      │   │   ├── auth.cpp
      │   │   └── auth.h
      │   └── ui/
      │       ├── ui.cpp
      │       └── ui.h
      ├── .gitignore
      ├── CHANGELOG.md
      ├── CMakeLists.txt
      ├── LICENSE.txt
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

- From `config.h`
    - `g_appTitle;` - global variable for application title
    - `g_loginTitle;`- global variable for login UI title
    - `g_errorTitle;` - global variable for error UI title
    - `g_fontName;` - global variable for font file name
    - `g_dataDirectory` - global variable for application data directory
    - `g_projectDirectory` - global variable for project directory
    - `g_dbNamePayroll;` - global variable for payroll database name
    - `g_dbNameTracker;` - global variable for monitoring database name
    - `g_dbNamePayroll` - global variable for payroll database name
    - `g_dbNameTracker` - global variable for monitoring database name
    - `g_defaultWidth;` - global variable for default window width
    - `g_defaultHeight;`- global variable for default window height
    - `g_loginWidth` - global variable for login window width
    - `g_loginHeight` - global variable for login window height
    - `g_errorWidth` - global variable for error window width
    - `g_errorHeight` - global variable for error window height

- From `ui.h`
    - `g_failedMessage` variable
        - holds the error message to be displayed in the failed UI
    - `constructUI()` function
        - sets up Hello ImGui
        - loads fonts
        - registers UIs

- From `db.h`
    - `createDatabase()` function
        - creates a new database
    - `openDatabase()` function
        - opens an existing database
    - `appendDatabase()` function
        - appends data to the database
    - `isSQLiteAvailable()` function
        - checks if SQLite is available

- From `system.h`
    - `fetchTime()` function
        - fetches the current system time
    - `logMessage()` function
        - logs a message to the system log
    - `createDirectory()` function
        - creates a new directory at the specified path
    - `deleteDirectory()` function
        - deletes the directory at the specified path
    - `createFile()` function
        - creates a new file at the specified path
    - `deleteFile()` function
        - deletes the file at the specified path

- From `auth.h`
    - `testAuth()` function
        - tests user authentication
    - `testDeployAuth()` function
        - tests deployment authentication
    - `adminAuth()` function
        - handles admin authentication
    - `basicAuth()` function
        - handles  user authentication

- From `cryptography.h`
    - `checkSodium()` function
        - checks if libsodium is available
    - `generateKey()` function
        - generates a cryptographic key
    - `toHex()` function
        - converts data to hexadecimal format
    - `hashKey()` function
        - hashes a password using Argon2
    - `saltKey()` function
        - generates a salt for hashing
    - `encryptFile()` function
        - encrypts data using symmetric encryption
    - `decryptFile()` function
        - decrypts data using symmetric encryption
    - `vault()` function
        - secures sensitive data

### Calling / Invoking Conventions
This section documents how modules in the repository should be invoked, the minimal contracts (inputs/outputs), common error modes, and a few examples. The conventions are based on the current repository layout and naming conventions (globals start with `g`, local variables with `l`, etc.).

1) General rules
   - Prefer calling public API functions declared in headers (e.g., `db::`, `system::`, `auth::`, `cryptography::`, `ui::`).
   - Always check return values and propagate errors to the UI via `g_failedMessage` or explicit out-parameters.
   - Keep UI rendering and business logic separate; UI files must not perform raw DB or crypto operations.
   - Do not modify off-limits UI internals (for example: `constructUI()` / internal `switchToUI()` handlers in `ui.cpp`).

2) Methods, functions or global variable/s (with conventions)
   - db:: (in `handler/db.cpp` / `handler/db.h`)
     - `db::isSQLiteAvailable()` -> bool
       - Returns true if SQLite is present and usable.
     - `db::createDatabase(const std::string &p_dbName)` -> bool
       - Creates or opens DB file. Returns true on success.
     - `db::openDatabase(const std::string &p_dbName)` -> bool
       - Opens existing DB. Caller handles errors.
     - `db::closeDatabase(const std::string &p_dbName)` -> bool
       - Closes DB handles, returns true on success.
     - `db::appendToDatabase(const std::string &p_dbName, const std::string &p_data)` -> bool
       - Appends data; prefer prepared statements and check SQLite return codes.

   - system:: (in `handler/system.cpp` / `handler/system.h`)
     - `system::createDirectory(const std::string &directoryName)` -> bool
       - Creates directory; treat "already exists" as success.
     - `system::fetchTime(PartDateTime part)` -> int
       - Returns requested date/time component.
     - `system::logMessage(const std::string &msg)` -> void
       - Log helpful debugging information before exposing shorter UI messages.

   - auth:: (in `security/auth.cpp` / `security/auth.h`)
     - `auth::authGateway(const std::string &username, const std::string &password, const std::string &source)` -> bool
       - Authenticates credentials; returns true on success and sets session state or returns false on failure.

   - cryptography:: (in `security/cryptography.cpp` / `security/cryptography.h`)
     - `cryptography::checkSodium()` -> bool
       - Verify libsodium is initialized and available.
     - `cryptography::generateKey(size_t length)` -> std::vector<unsigned char>
       - Generate a key of requested length. Do not enforce an arbitrary internal limit; return empty vector on invalid request.
     - `cryptography::saltKey()` -> std::vector<unsigned char>
       - Generate and return a secure salt for password hashing.
     - `cryptography::hashKey(const std::string &password, const std::vector<unsigned char> &salt)` -> std::string
       - Use Argon2 to produce a hash; return encoded string (hex/base64).
     - `cryptography::encryptFile(const std::string &filepath, const std::vector<unsigned char> &key)` -> bool
       - Encrypt file; writes `filepath + ".enc"` and returns true on success.
     - `cryptography::decryptFileInPlace(const std::string &encPath, const std::vector<unsigned char> &key)` -> bool
       - Decrypt and replace the encrypted file (in-place). On failure, leave encrypted file intact.

   - ui:: (in `ui/ui.cpp` / `ui/ui.h`)
     - `constructUI(const std::string &title, const std::string &fontLocation, int widthPx, int heightPx, const std::string &assetsFolder)` -> void
       - Register UI handlers and load fonts/resources. Keep UI handlers small and free of heavy business logic.

3) Example call flows
   - Startup (main.cpp):
     - Load config globals (`g_*`).
     - Validate critical dependencies: `db::isSQLiteAvailable()` and `cryptography::checkSodium()`; if missing, set `g_failedMessage` and show failed UI.
     - Ensure app data directory via `system::createDirectory()`.
     - Open or create DB via `db::createDatabase()`.
     - Call `constructUI()` and start the UI loop.

   - Login flow (UI handler):
     - If `username.empty() || password.empty()`:
       - set `g_failedMessage = "Empty credentials"` and switch to `failedUI()`.
     - Else: call `auth::authGateway(username, password, "loginUI")`.
       - On success: switch to main UI. On failure: set `g_failedMessage` and show failed UI.

4) Error and edge-case handling (recommended patterns)
   - Log detailed errors via `system::logMessage()` before setting short, user-facing `g_failedMessage`.
   - For filesystem operations, treat "already exists" as success unless replacement is intended.
   - For DB operations, prefer prepared statements and validate inputs.
   - For cryptography: validate key sizes and return empty/false results on invalid parameters; callers should translate to UI errors.

5) Minimal examples
   - Directory creation:

         std::string path = "/path/to/appdata";
         if (!system::createDirectory(path)) {
             system::logMessage("createDirectory failed: " + path);
             g_failedMessage = "Failed to prepare application data directory";
             // switch to failed UI
         }

   - Login handler (conceptual):

         if (username.empty() || password.empty()) {
             g_failedMessage = "Empty credentials";
             // switch to failed UI
         } else if (!auth::authGateway(username, password, "loginUI")) {
             g_failedMessage = "Authentication failed";
             // switch to failed UI
         }

6) Quick checklist for contributors when calling modules
   - Read the header of the module you call and prefer the public API.
   - Check return codes and propagate errors to the caller/UI.
   - Avoid modifying `ui.cpp` internals; file issues/PRs for UI changes.


## Build & Run (Desktop)

### Prerequisites (For Development Computer Only)
- C++20 or later
> - It should be at C++26 but Apple Clang partially support C++26 
- CMake 3.22 or later
- sqlite3 3.50.4
- Hello ImGui

### Automated Builds (CI/CD)

- This project uses GitHub Actions for continuous integration and provides both automated and manual build workflows across multiple platforms.
- For detailed CI/CD documentation, build configuration, and troubleshooting, see [BUILD.md](.github/BUILD.md).

#### Manual Build

- For platform-specific configurations and legacy support, we provide **five manual-trigger workflows**:

#### Windows Builds

Windows README files:
- [README-windows-x86_64.md](.github/workflows/README-windows-x86_64.md)
- [README-windows-arm64.md](.github/workflows/README-windows-arm64.md)

| Workflow    | Target Hardware          | Architecture | Target Windows Version | C++ Std | Asset Name                                             |
|-------------|--------------------------|--------------|------------------------|---------|--------------------------------------------------------|
| **Windows** | Computer with Windows 10 | x86_64       | Windows 10             | C++20   | `structuracost-windows-x86_64-${version}-${dev_stage}` |
| **Windows** | Computer with Windows 10 | ARM64        | Windows 10 for ARM     | C++20   | `structuracost-windows-arm64-${version}-${dev_stage}`  |
| **Windows** | Computer with Windows 11 | x86_64       | Windows 11             | C++20   | `structuracost-windows-x86_64-${version}-${dev_stage}` |
| **Windows** | Computer with Windows 11 | ARM64        | Windows 11 for ARM     | C++20   | `structuracost-windows-arm64-${version}-${dev_stage}`  |

#### macOS Builds

macOS README files:
   - [README-macos-arm64.md](.github/workflows/README-macos-arm64.md)
   - [README-macos-x86_64.md](.github/workflows/README-macos-x86_64.md)

| Workflow  | Target Hardware             | Architecture | Target macOS Versions | C++ Std | Asset Name                                        |
|-----------|-----------------------------|--------------|-----------------------|---------|---------------------------------------------------|
| **macOS** | Apple Silicon (M1/M2/M3/M4) | ARM64        | macOS Sequoia 15.0    | C++20   | `structuracost-mac-arm64-${version}-${dev_stage}` |
| **macOS** | Intel Macs                  | x86_64       | macOS Ventura 13.0    | C++20   | `structuracost-mac-intel-${version}-${dev_stage}` |

#### Linux Builds

Linux README files:
   - [README-linux.md](.github/workflows/README-linux.md)
   - [README-linux-rpm.md](.github/workflows/README-linux-rpm.md)
   - [README-linux-suse.md](.github/workflows/README-linux-suse.md)

| Workflow                    | Target Hardware     | Architecture | Target Linux Versions | C++ Std | Asset Name                                                 |
|-----------------------------|---------------------|--------------|-----------------------|---------|------------------------------------------------------------|
| **Linux (Debian/Ubuntu)**   | x86_64 Computers    | x86_64       | Ubuntu 20.04          | C++20   | `structuracost-linux-debian-intel-${version}-${dev_stage}` |
| **Linux (Debian/Ubuntu)**   | ARM64 Computers     | ARM64        | Ubuntu 20.04 (ARM64)  | C++20   | `structuracost-linux-debian-arm64-${version}-${dev_stage}` |
| **Linux (Fedora/Red Hat)**  | x86_64 Computers    | x86_64       | Fedora Latest         | C++20   | `structuracost-linux-rpm-intel-${version}-${dev_stage}`    |
| **Linux (Fedora/Red Hat)**  | ARM64 Computers     | ARM64        | Fedora Latest (ARM64) | C++20   | `structuracost-linux-rpm-arm64-${version}-${dev_stage}`    |
| **Linux (SUSE Family)**     | x86_64 Computers    | x86_64       | openSUSE Tumbleweed   | C++20   | `structuracost-linux-suse-intel-${version}-${dev_stage}`   |
| **Linux (SUSE Family)**     | ARM64 Computers     | ARM64        | openSUSE Tumbleweed   | C++20   | `structuracost-linux-suse-arm64-${version}-${dev_stage}`   |


**To trigger manual workflows:** Go to Actions → Select workflow → Run workflow


#### Release Builds (All Platforms)

| Workflow                  | Platforms          | Trigger                    |
|---------------------------|--------------------|----------------------------|
| **Release Build**         | All supported      | Git tag (e.g., `v1.0.0`)   |

- When a version tag is created, the **Release Build** workflow automatically:
  - Builds binaries for all supported platforms (Linux x64, linux ARM64, Windows x64, Windows ARM64, macOS ARM64, macOS Intel, macOS Legacy)
  - Creates a GitHub Release with the tag
  - Attaches all platform binaries to the release as downloadable assets

#### Downloading Pre-built Binaries

#### From GitHub Actions (Automated Builds)

For the latest builds from the `master` branch:

  1. Navigate to the **Actions** tab in the repository
  2. Click on the latest successful workflow run (look for green checkmark)
  3. Scroll to the **Artifacts** section at the bottom of the page
  4. Download the artifact for your platform
  5. Extract the archive and run the executable

**Note:** Artifacts expire after 90 days. For permanent builds, use releases (see below).

#### From Manual Builds

For platform-specific or legacy builds:

1. Navigate to the **Actions** tab
2. Select the appropriate workflow from the left sidebar:
    - `macOS`
    - `Windows`
    - `Linux (Debian/Ubuntu)`
    - `Linux (Fedora/Red Hat)`
    - `Linux (SUSE Family)`
3. Click **Run workflow** button (top right)
4. Configure build options if prompted
5. Wait for the build to complete (status changes to green checkmark)
6. Download the artifact from the workflow run page:
    - `structuracost-macos-arm64.tar.gz` (Apple Silicon)
    - `structuracost-macos-x64.tar.gz` (Intel macOS)
    - `structuracost-linux-x64.tar.gz` (Linux x64)
    - `structuracost-windows-x64.zip` (Windows x64)
7. Extract and run the executable

#### From Releases (Tagged Versions) — Recommended

For release build (version tagging and automatic release):

1. Navigate to the **Actions** tab
2. Select the `Release` workflow from the left sidebar 
3. Select the desired version (e.g., `v1.0.0`)
4. Download the binary for your platform from the **Assets** section:
    - `structuracost-linux-x64.tar.gz`
    - `structuracost-windows-x64.zip`
    - `structuracost-macos-arm64.tar.gz`
    - `structuracost-macos-x64.tar.gz`
    - `structuracost-macos-legacy.tar.gz`
5. Extract and run the executable

**Note:** Release binaries are permanent and recommended for production use.

## Build and Run (Mobile)

- There is a separate <a href="https://github.com/joshua-9402/cpet-140-final-project-mobile"> repository </a> for the Android and iOS version with appropriate setup and build instructions for both Android and iOS.


## Dependencies

- sqlite3 (runtime + development headers)
- libsodium (for cryptography)
- Hello Dear ImGui 
- Optional:
  - libraries for platform windows/rendering (GLFW/SDL/DirectX/Metal)


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
