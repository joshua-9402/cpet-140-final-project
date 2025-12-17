<p align="center"><img src="assets/icons/app_icon.png" alt="app icon" width="128" style="display:block;margin:0 auto 6px;" /></p>
<h1 align="center" style="margin-top:0.0rem;">StructuraCost</h1>

<h6 align="center">This repository is the final project for CpET 140—Computer Programming 1 </h6>

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
    - [Modules & Interfaces](#modules--interfaces)
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
  - [Dependencies](#dependencies)

III. Quality & Contributions
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
- Target platforms: Desktop (Windows/macOS/Linux)

- Recent repository updates: several small, backwards-compatible fixes and CI improvements were applied to keep builds stable across platforms (notably: `system::createDirectory` now returns `bool`, `system::appShutdown` uses a portable partial_sort variant, and cryptography helper types were clarified to use byte vectors for keys/salts and in-place decryption semantics). For full details and troubleshooting steps (libsodium, freetype, Windows linking, macOS toolchain), see the "Troubleshooting & Debugging Tips" section below.

### Repository Structure
      cpet-140-final-project/
      ├── .github/
      │   └── workflows/
      │       ├── BUILD.md
      │       ├── README-linux.md
      │       ├── README-linux-rpm.md
      │       ├── README-linux-suse.md
      │       ├── README-macos-arm64.md
      │       ├── README-macos-intel.md
      │       ├── README-windows-arm64.md
      │       ├── README-windows-x86_64.md
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
      │       ├── app_icon.png
      │       └── user_icon.png
      ├── assets (raw)/
      │   ├── user_icon.psd
      │   └── concept.png
      ├── dependencies/
      │   └── sqlite/
      │      ├── sqlite3.c
      │      └── sqlite3.h
      ├── doc/
      │   ├── system_flowchart/
      │   │   └── CpET 140 Final Project System Flowchart.svg
      │   ├── CpET 140 - Computer Programming I - Final Output.pdf
      │   ├── modules/
      │   └── hello_imgui_manual.pdf
      ├── src/
      │   ├── main.cpp
      │   ├── config/
      │   │   ├── config.cpp
      │   │   └── config.h
      │   ├── handler/
      │   │   ├── db.cpp
      │   │   ├── db.h
      │   │   ├── print.cpp
      │   │   ├── print.h
      │   │   ├── system.cpp
      │   │   └── system.h
      │   ├── core/
      │   │   ├── monitor.cpp
      │   │   ├── monitor.h
      │   │   ├── payroll.cpp
      │   │   └── payroll.h
      │   ├── security/
      │   │   ├── cryptography.h
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

### Modules & Interfaces

Below each module is a concise responsibility summary, a short list of public functions/globals other modules call, and a link to the detailed, code-accurate module doc in `doc/modules/`.

- `main.cpp`
  - Responsibilities: program entry, startup diagnostics, dependency checks, and starting the UI runner via `ui::constructUI()`.
  - Public interface: `main()` (startup orchestration)
  - See: `doc/modules/` (startup flow is covered across the module docs)

- `src/config/config.h` / `src/config/config.cpp` (`appConfig`)
  - Responsibilities: global application configuration variables (titles, default paths, DB filenames, window sizes).
  - Public interface (globals): `appConfig::g_auth`, `appConfig::g_testMode`, `appConfig::g_appTitle`, `appConfig::g_loginTitle`, `appConfig::g_errorTitle`, `appConfig::g_fontName`, `appConfig::g_dataDirectory`, `appConfig::g_projectDirectory`, `appConfig::g_payrollDirectory`, `appConfig::g_projectExpenseDirectory`, `appConfig::g_payrollAttendanceDirectory`, `appConfig::g_dbNamePayroll`, `appConfig::g_dbNameProject`, `appConfig::g_defaultWidth`, `appConfig::g_defaultHeight`, `appConfig::g_loginWidth`, `appConfig::g_loginHeight`, `appConfig::g_errorWidth`, `appConfig::g_errorHeight`.
  - Full doc: [doc/modules/config-config.md](doc/modules/config-config.md)

- `src/ui/ui.h` / `src/ui/ui.cpp` (`ui`)
  - Responsibilities: immediate-mode UI using HelloImGui; registry of named UIs, font/assets loading, and main two-column layout (selector / active panel).
  - Public interface: `ui::g_failedMessage`, `ui::g_userName`, `ui::g_position`, `ui::constructUI(const std::string&, const std::string&, int, int, const std::string&)`.
  - Notes: UI handlers should call into `handler/*` and `security/*` APIs rather than performing raw DB/crypto work.
  - Full doc: [doc/modules/ui-ui.md](doc/modules/ui-ui.md)

- `src/handler/db.h` / `src/handler/db.cpp` (`db`)
  - Responsibilities: lightweight SQLite helpers tailored to the project's payroll and projects schemas.
  - Public interface: `db::isSQLiteAvailable()`, `db::createDatabase(const std::string&)`, `db::openDatabase(const std::string&)`, `db::closeDatabase()`, `db::appendDatabase(const std::string&, const std::string&)`, `db::updateDatabase(const std::string&, const std::string&, const std::string&)`, `db::deleteRow(const std::string&, const std::string&)`, `db::fetchCell(const std::string&, size_t, size_t)`, `db::checkEmployeeChanges()`, `db::rearrangeEmployeeIDs()`, `db::checkProjectChanges()`, `db::rearrangeProjectIDs()`.
  - Security note: current implementation builds SQL via string concatenation; prefer prepared statements for user data.
  - Full doc: [doc/modules/handler-db.md](doc/modules/handler-db.md)

- `src/handler/system.h` / `src/handler/system.cpp` (`system`)
  - Responsibilities: cross-platform filesystem helpers, time utilities, logging, file/directory creation/removal, copy, and shutdown/backup rotation.
  - Public interface: `system::fetchTime(PartDateTime)`, `system::timeDateString()`, `system::logMessage(messageClassification, const std::string&)`, `system::createDirectory(const std::string&)`, `system::searchDirectory(const std::string&)`, `system::copyDirectory(const std::string&, const std::string&)`, `system::deleteDirectory(const std::string&)`, `system::createFile(const std::string&)`, `system::searchFile(const std::string&)`, `system::deleteFile(const std::string&)`, `system::printPayslips(const std::string&, const std::string&, const std::vector<int>&)`, `system::printProjectReport(const std::string&, const std::string&)`, `system::openFileInBrowser(const std::string&)`, `system::appShutdown()`.
  - Full doc: [doc/modules/handler-system.md](doc/modules/handler-system.md)

- `src/handler/print.h` / `src/handler/print.cpp` (`print`)
  - Responsibilities: project report and payslip HTML generation and printing helpers used by the UI and system.
  - Full doc: [doc/modules/handler-print.md](doc/modules/handler-print.md)

- `src/security/auth.h` / `src/security/auth.cpp` (`auth`)
  - Responsibilities: small, in-source demo/test authentication helpers used by the UI for test/admin flows.
  - Public interface: `auth::testAuth()`, `auth::testDeployAuth()`, `auth::adminAuth()`, `auth::mainAuth()`.
  - Warning: these are demonstrational checks and mutate UI globals; replace it with a secure auth backend for production.
  - Full doc: [doc/modules/security-auth.md](doc/modules/security-auth.md)

- `src/security/cryptography.h` / `src/security/cryptography.cpp` (`cryptography`)
  - Responsibilities: libsodium wrapper for initialization, hashing, key generation, to-hex conversion, password/key salting, file encryption/decryption, and a small append-only vault.
  - Public interface: `cryptography::checkSodium()`, `cryptography::hashKey(const std::string&, int)`, `cryptography::generateKey(size_t)`, `cryptography::toHex(const std::vector<unsigned char>&)`, `cryptography::saltKey(const std::string&)`, `cryptography::encryptFile(const std::string&, const std::vector<unsigned char>&)`, `cryptography::decryptFile(const std::string&, const std::vector<unsigned char>&, std::string*)`.
  - Important behavior: `encryptFile` writes `filepath + ".enc"` with `salt||nonce||ciphertext`; `decryptFile` overwrites the input path with plaintext on success (keep backups).
  - Full doc: [doc/modules/security-cryptography.md](doc/modules/security-cryptography.md)

- `src/core/` (payroll, monitor)
  - Responsibilities: domain logic for payroll calculations and project expense monitoring; call `db` helpers for persistence and produce data for the UI.
  - Integration guidance: see the module docs above and the `src/core/` code comments.


### Calling / Invoking Conventions
This section documents calling conventions, minimal contracts, common error modes, and examples. Follow these conventions to keep code consistent and robust.

1) General rules
   - Use public APIs declared in headers (`db::`, `system::`, `auth::`, `cryptography::`, `ui::`).
   - Always check return values; bubble errors to the UI via `ui::g_failedMessage` and log details with `system::logMessage()`.
   - Keep UI handlers free of heavy logic (no direct DB writes or long-running crypto operations).
   - Avoid editing `ui.cpp` internals (e.g., `constructUI()` and internal handler registry) — file issues / PRs for requested changes.

2) Key function conventions (examples)
   - `db::isSQLiteAvailable()` -> bool — Check before using DB APIs.
   - `db::createDatabase(p_dbName)` -> bool — Create schema by known filename; returns false for unrecognized names or errors.
   - `db::appendDatabase(p_dbName, p_data)` -> bool — Append row; `p_data` must be a correctly quoted comma-separated SQL value list.
   - `db::updateDatabase(p_dbName, p_id, p_data)` -> bool — Update row by id; `p_data` should be a valid SQL SET clause. Prefer prepared statements for user-supplied data.
   - `system::createDirectory(path)` -> bool — Create parents as needed; treat already-existing as success.
   - `system::searchFile(path)` / `system::searchDirectory(path)` -> bool — Existence checks with safe fallbacks.
   - `cryptography::generateKey(bits)` -> std::vector<unsigned char> — Request key lengths that meet implementation limits (multiple of 8; within supported range).
   - `cryptography::encryptFile(path, key)` / `cryptography::decryptFile(path, key, &err)` -> bool — Check returned bool and inspect `err` text if provided. `decryptFile` overwrites files in-place on success.

3) Example call flows
   - Startup:
     - Validate cryptography and DB availability, ensure `data/` directory exists, create DB if missing, then start UI.

   - Login UI handler:
     - If username or password is empty: set `ui::g_failedMessage` and open failed modal.
     - Otherwise, call `auth::testAuth` / `auth::testDeployAuth` or your real auth backend; on success set `appConfig::g_auth` and proceed.

4) Error & defensive patterns
   - Log verbose errors with `system::logMessage()` and present concise, user-friendly messages in `ui::g_failedMessage`.
   - Prefer non-throwing overloads (`std::filesystem` with `std::error_code`) for production code paths.
   - Validate and sanitize any data passed into SQL construction; migrate to prepared statements when possible.

5) Quick checklist for contributors
   - Read the header for the module you call and prefer the public API.
   - Check return codes, log internally, and surface short messages to the UI.
   - Avoid changing `ui.cpp` internals; file issues or PRs for UI changes.


## Build & Run (Desktop)

### Prerequisites (For Development Computer Only)
- C++20 or later
> - It should be at C++26 but Apple Clang partially supports C++26 
- CMake 3.22 or later
- sqlite3 3.50.4
- Hello ImGui

### Automated Builds (CI/CD)

- This project uses GitHub Actions for continuous integration and provides both automated and manual build workflows across multiple platforms.
- For detailed CI/CD documentation, build configuration, and troubleshooting, see [BUILD.md](.github/workflows/BUILD.md).

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
   - [README-macos-intel.md](.github/workflows/README-macos-intel.md)

| Workflow  | Target Hardware             | Architecture | Target macOS Versions | C++ Std | Asset Name                                        |
|-----------|-----------------------------|--------------|-----------------------|---------|---------------------------------------------------|
| **macOS** | Apple Silicon (M1/M2/M3/M4) | ARM64        | macOS Sequoia 15.0    | C++20   | `structuracost-mac-arm64-${version}-${dev_stage}` |
| **macOS** | Intel Macs                  | x86_64       | macOS Sequoia 15.0    | C++20   | `structuracost-mac-intel-${version}-${dev_stage}` |

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
| **Release**               | Selected platforms | Manual (Actions → Release) |

- When manually triggered, the **Release** workflow:
  - Computes the version from inputs (stage, iteration, W.X.Y.Z)
  - Builds binaries for the selected platforms/architectures (Linux x64/ARM64, Windows x64/ARM64, macOS ARM64/Intel)
  - Creates a GitHub Release and attaches the built artifacts as downloadable assets

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
3. Click the **Run workflow** button (top right)
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
3. Configure the desired version and targets, then run the workflow
4. Download the binary for your platform from the **Assets** section:
    - `structuracost-linux-x64.tar.gz`
    - `structuracost-windows-x64.zip`
    - `structuracost-macos-arm64.tar.gz`
    - `structuracost-macos-x64.tar.gz`
    5. Extract and run the executable

**Note:** Release binaries are permanent and recommended for production use.

## Dependencies

- sqlite3 (runtime + development headers)
- libsodium (for cryptography)
- Hello ImGui 
- Optional:
  - libraries for platform windows/rendering (GLFW/SDL/DirectX/Metal)


## Contribution Guidelines
The participation of everyone is needed to make this project a success. Please follow the guidelines below when contributing to this project.

> Notes:
> - `ui.cpp`, `ui.h`, `db.cpp` and `db.h` are OFF LIMITS especially for `constructUI()` and the internal UI switching logic (in `ui.cpp`).
>   - For bugs, please file an issue instead.
>   - For enhancements, please discuss with the maintainer first or file an issue regarding the enhancement.
>   - For adding new UIs, please file an issue first to discuss the addition.
> - When contributing code, please ensure that your code adheres to the coding conventions outlined below.
> - Any significant deviations from the coding conventions should be explained in the PR / commit description, or it will be rejected.
> - The use of other languages other than English is not allowed unless specified.

### Whitespace and Formatting
- Use four spaces or one tab for indentation.
- Limit lines to a maximum of 20 words (comments only) for better readability.
  - This restriction applies only to inline comments within source files
  - Header comments, top-of-function/method comments, and documentation files are exempted.
- Use blank lines to separate logical sections of code.
  - For spaces in function / methods:
    - give two lines for each function / method definition and implementation.

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
    - `build` - changes to a build process or dependencies
      - this is different from `chore` since it directly affects the build process or dependencies
      - this option is only available for `CMakeLists.txt` or build scripts
    - the only exception is `initial commit` and the few later commits in the repository and `README.md`
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
  - `bug` - nothing is working as expected
  - `documentation` - improvements or additions to documentation
  - `enhancement` - new feature or request
  - `help wanted` - assistance needed
  - `question` - request for information or clarification
- Reference related issues or PRs when applicable.


### Filing Pull Requests
- Ensure your branch is up to date with the main branch before creating a PR.
- Provide a clear title and description of the changes made.
- Reference any related issues.
- Follow the project's coding conventions and guidelines. Any significant deviations should be explained in the PR description, or it will be rejected.
- Ensure all tests pass before submitting the PR. Submitted PRs will undergo for a review and also include the test results.

### Troubleshooting & Debugging Tips
This section highlights common issues seen in local and CI builds and concrete fixes that reflect recent changes.

1) libsodium / cryptography issues
   - Symptom: Linker errors (undefined symbols like `sodium_init`, `crypto_aead_xchacha20poly1305_ietf_encrypt`, etc.) or runtime `crypto` failures.
   - Fixes:
     - Ensure libsodium is installed for the platform and matches the target architecture (x64 vs. ARM64). In CI add a step to install libsodium BEFORE CMake configure. Example (GitHub Actions):

       - For Linux (Debian/Ubuntu): `sudo apt-get install -y libsodium-dev`
       - For macOS (Homebrew): `brew install libsodium`
       - For Windows: download the proper prebuilt libsodium matching your MSVC toolset and place in `dependencies/libsodium` or install via vcpkg with the correct triplet.

     - When linking on macOS, ensure the linker can find the `libsodium.dylib` (use -L/opt/homebrew/lib and set rpath if necessary). The CMake configuration already attempts to find libsodium; adjust runner environment if the library is installed in a non-default location.
     - On Windows ensure the libsodium lib's machine type and runtime (v142/v143) match the build target. Use the proper libsodium subfolder (x64/ARM64/Win32) in `dependencies/libsodium` if bundling the library.

2) Freetype version failing in CI
   - Symptom: CMake: "Could NOT find Freetype: Found unsuitable version ... but required is at least 2.12"
   - Fix: Install or provide a newer Freetype for the runner (use distro package or build a compatible version in CI). For Debian/Ubuntu use a newer image or build Freetype from source as a pre-step in CI.

3) File API / system changes
   - `system::createDirectory()` changed return type to `bool`. Callers were updated to build paths as strings and call `searchDirectory()` and `createDirectory()` separately (see `main.cpp` for example). If you encounter compile errors complaining about binding a bool to a string, update your call-site to use an explicit path string and not pass the function call as a parameter where a string is expected.

4) Backup cleanup and sorting
   - `system::appShutdown()` now uses a portable `std::partial_sort` comparator instead of `std::ranges::partial_sort` to maintain compatibility with older standard library versions used in some CI environments.

5) Windows linking / architecture mismatches
   - Symptom: LNK2019 unresolved external symbols or machine-type conflicts (warnings like library machine type 'ARM64' conflicts with target machine type 'x64').
   - Fix: Use libsodium build artifacts that match the MSVC target architecture and toolset chosen by the CI workflow. If using `dependencies/libsodium`, ensure you add the correct subfolder (Win32/x64/ARM64) with matching runtime and MSVC version.

6) Decryption / encryption behavior
  - Decryption in-place: `cryptography::decryptFile()` will replace the encrypted file with the decrypted version on success. If decryption fails, the encrypted file remains untouched. Check the logs (use `system::logMessage`) for detailed reasons. Key generation returns a vector, and functions validate key size; errors like "Key generation failed or wrong size" mean the caller passed an invalid size — adjust caller or remove artificial limits.

7) UI runtime issues (selector / quick disappearing windows)
   - Symptom: clicking UI buttons briefly changes the view but reverts instantly, or secondary windows (payroll/monitor) appear to then disappear.
   - Checklist to debug:
     - Verify `g_currentUI`/UI state is set and not overwritten every frame by `constructUI()` or by a later UI handler call.
     - Make sure login handlers populate `appConfig::g_auth` only on successful authentication and the main loop in `main.cpp` respects that flag.
     - For selector toggles, ensure you update existing DB rows (UPDATE) instead of always INSERT-ing duplicates. The core CRUD functions were updated to support modify/update flows — prefer using them.

8) General advice when debugging builds
   - Re-run the failing CI job locally if possible (same base image and packages). Reproducing CI locally makes it easier to iterate.
   - When fixing a linker error, examine which library provides the missing symbol and confirm that the correct architecture and runtime variant are used.
   - Log internal errors using `system::logMessage()` before setting `g_failedMessage` to keep user facing errors short.
