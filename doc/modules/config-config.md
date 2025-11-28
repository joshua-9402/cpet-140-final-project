# Configuration module (`config.h` / `config.cpp`)

## Purpose

The `config` module declares and defines a small set of global application configuration variables used throughout the codebase. It is not a JSON-backed configuration manager — instead the project uses a single `appConfig` class with static members that hold runtime constants and paths (titles, default directories, window sizes, and simple feature flags).

## Files

- `src/config/config.h` — declares the `appConfig` class and its static members
- `src/config/config.cpp` — defines the static members and provides default values

## Public API (summary)

The module exposes public static members via `appConfig::`:

- `bool appConfig::g_auth` — global authentication flag (default: false)
- `bool appConfig::g_testMode` — test-mode flag (default: false)
- `std::string appConfig::g_appTitle` — application title
- `std::string appConfig::g_loginTitle` — login window title
- `std::string appConfig::g_errorTitle` — error window title
- `std::string appConfig::g_fontName` — default font path
- `std::string appConfig::g_dataDirectory` — data directory (relative)
- `std::string appConfig::g_projectDirectory` — projects directory (relative)
- `std::string appConfig::g_payrollDirectory` — payroll directory (relative)
- `std::string appConfig::g_projectExpenseDirectory` — expense directory (relative)
- `std::string appConfig::g_dbNamePayroll` — payroll DB filename
- `std::string appConfig::g_dbNameProject` — project DB filename
- `int appConfig::g_defaultWidth`, `g_defaultHeight` — default window size
- `int appConfig::g_loginWidth`, `g_loginHeight` — login window size
- `int appConfig::g_errorWidth`, `g_errorHeight` — error dialog size

There are no getters/setters; code reads/writes these static members directly.

## Data shapes

- Simple scalar types: `bool`, `int`, `std::string`.
- Values are intended as small, read-mostly runtime constants; they can be mutated at startup if needed.

## Error modes and return conventions

- This module does not return errors — it's a passive container. Any validation or I/O (e.g., creating directories using these paths) must be performed by callers.

## Security considerations

- These globals may include filesystem paths. Do not store secrets (passwords, private keys) in these variables.
- When composing filesystem paths from these values, validate or sanitize user input before combining.

## Build / runtime dependencies

- No external dependencies. Uses the C++ standard library only. The variables are defined in `config.cpp`; ensure that file is compiled and linked once to avoid duplicate symbol errors.

## Usage examples

```cpp
#include "config/config.h"

void example() {
    // Read a path
    std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    // Toggle test mode at startup
    appConfig::g_testMode = true;

    // Change UI titles
    appConfig::g_appTitle = "StructuraCost";
}
```

## Troubleshooting & debugging tips

- Duplicate symbol linker errors for the `appConfig` members typically mean `config.cpp` was not linked or the static definitions were accidentally redefined. Make sure only `config.cpp` defines the variables and headers only declare them.
- If you change a default value in `config.cpp`, rebuild the project to ensure the updated symbol is linked.
- Be mindful of modifying these globals concurrently from multiple threads; they are not synchronized by the module itself.

---

## Repository sync note

Updated to reflect the actual implementation in `src/config/config.h` and `src/config/config.cpp` (sync date: 2025-11-29). This file replaces the previous JSON-driven description to match the code.
