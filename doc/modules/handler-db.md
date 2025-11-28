# Changelog

## [Unreleased]
- No changes yet.

---

# Database handler module (`handler/db.h` / `handler/db.cpp`)

## Purpose

The `handler-db` module provides a small persistence adapter using SQLite. It exposes a tiny API for creating/opening simple application databases, appending rows, updating rows, and a helper to check whether SQLite is available in the runtime environment.

This module is intentionally minimal and is intended for the project's payroll and project-tracking databases; it is not a full-featured ORM.

## Files

- `src/handler/db.h` — public declarations
- `src/handler/db.cpp` — implementation (uses the bundled `sqlite3` C API and `appConfig` values from `config`)

## Public API (summary)

- `static bool db::isSQLiteAvailable()`
  - Returns `true` if the embedded SQLite library appears to be present and usable. Performs a quick `sqlite3_libversion()` check and tries to open an in-memory database.

- `static bool db::createDatabase(const std::string& p_dbName)`
  - Creates (or opens) the database file `p_dbName` and initializes the schema for known database filenames. Supported database files (by name) are:
    - `appConfig::g_dbNamePayroll` (creates `EMPLOYEES` table)
    - `appConfig::g_dbNameProject` (creates `PROJECT_LIST` table)
  - Returns `true` on success, `false` on error or if the filename is not recognized.

- `static bool db::openDatabase(const std::string& p_dbName)`
  - Attempts to open the database file `p_dbName` (read-only test open). Returns `true` when the database can be opened.

- `static bool db::appendDatabase(const std::string& p_dbName, const std::string& p_data)`
  - Appends a new row into the table associated with `p_dbName`. `p_data` should be a comma-separated SQL values list matching the target table's column order (for example: `"'John Doe', 'Manager', 'Manila', 100.0, 160.0, 0.0'`).
  - Supported targets:
    - Payroll: inserts into `EMPLOYEES (NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE)`
    - Projects: inserts into `PROJECT_LIST (PROJECT_NAME, STATUS, START_DATE)`
  - Returns `true` on success.

- `static bool db::updateDatabase(const std::string& p_dbName, const std::string& p_id, const std::string& p_data)`
  - Updates an existing row by id in the table associated with `p_dbName`.
  - `p_data` contains the SQL `SET` clause contents (e.g., `"NAME='Jane Doe', SALARY=150.0"`).
  - Note: the implementation currently constructs the `WHERE` clause using column names `ID` or `ProjectID` depending on the database and string-concatenates SQL; callers must ensure `p_id` and `p_data` are valid and sanitized.
  - Returns `true` on success.

## Data shapes

- `p_dbName`: file path or filename string (the implementation normalizes to the filename when deciding which schema to create)
- `p_data` (append): comma-separated SQL values string wrapped appropriately in quotes for text fields
- `p_id`: identifier string (should be numeric id value matching table primary key)
- All functions return `bool` indicating success or failure; no higher-level result objects are returned.

## Error modes and return conventions

- Functions return `false` on failure; callers should check return values and can inspect SQLite error messages when debugging.
- `createDatabase` returns `false` when the filename is not one of the supported recognized database names.
- The module uses `sqlite3_exec` with dynamically constructed SQL strings — errors cause `sqlite3_exec` to return non-OK results and the function to return `false`.

## Security considerations

- The current implementation constructs SQL by concatenating strings (both for INSERT and UPDATE). This is vulnerable to SQL injection if untrusted inputs are passed directly into `p_data` or `p_id`.
  - Recommendation: switch to prepared statements (`sqlite3_prepare_v2`, `sqlite3_bind_*`, `sqlite3_step`) to safely bind parameters.

- Never pass raw user input directly into `p_data`/`p_id` without validation/escaping.

- Database files may contain sensitive data; store encrypted backups and restrict file permissions.

## Build / runtime dependencies

- Uses the bundled `sqlite3.h`/`sqlite3.c` (project includes `dependencies/sqlite`) and links against the compiled sqlite3 object. No additional runtime dependency should be required.
- Uses `appConfig` constants from `src/config/config.h` to decide which schema to create/open (e.g., `appConfig::g_dbNamePayroll`).

## Usage examples

Create and open the payroll DB:

```cpp
#include "handler/db.h"
#include "config/config.h"

// Create DB (initializes schema if not present)
if (!db::createDatabase(appConfig::g_dbNamePayroll)) {
    // handle error
}

// Open DB to check it exists
if (!db::openDatabase(appConfig::g_dbNamePayroll)) {
    // handle error
}

// Append a new employee (values must match table columns order)
std::string values = "'John Doe', 'Manager', 'Manila', 100.0, 160.0, 0.0";
if (!db::appendDatabase(appConfig::g_dbNamePayroll, values)) {
    // handle error
}

// Update an employee: p_id is the numeric id and p_data is SQL SET clause
if (!db::updateDatabase(appConfig::g_dbNamePayroll, "1", "NAME='John Smith', SALARY=120.0")) {
    // handle error
}
```

## Troubleshooting & debugging tips

- If `isSQLiteAvailable()` returns `false`:
  - Verify `sqlite3_libversion()` is accessible and the in-memory open test succeeds. If the function fails in CI, ensure `dependencies/sqlite` is compiled and linked.

- If `createDatabase` returns `false` unexpectedly:
  - Ensure the `p_dbName` filename matches `appConfig::g_dbNamePayroll` or `appConfig::g_dbNameProject` (the function recognizes the DB by filename and builds the corresponding CREATE TABLE statement).
  - Check that the process has file system write permissions in the target directory.

- If `appendDatabase`/`updateDatabase` fail:
  - Print or log the constructed SQL string and run it in `sqlite3` client to see the exact error.
  - Ensure `p_data` is correctly quoted for text values and matches the expected number/order of columns.

- If you need more advanced queries or parameter binding, refactor this module to use `sqlite3_prepare_v2` and parameter binding APIs.

---

## Repository sync note

Updated to reflect the actual implementation in `src/handler/db.h` and `src/handler/db.cpp` (sync date: 2025-11-29). This doc replaces a prior generic DB doc with the exact API and usage patterns present in the codebase.
