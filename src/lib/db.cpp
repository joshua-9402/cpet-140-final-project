/*
* CpET 140 Final Project — Database module
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Provides persistence adapters used by POS/Inventory (e.g., load/save items,
 *   record sales, read/write settings) via APIs declared in db.h.
 *
 * Boundaries
 * - No rendering and no direct UI calls. Storage details are hidden behind db.h,
 *   allowing stubs/mocks or different backends.
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if used from workers.
 * - Prefer explicit status results and avoid partial writes on failure.
 */

#include "db.h"
#include <sqlite3.h>

// Check availability by opening and closing an in-memory database.
// Possible outputs: true (OK), false (failed to open).
bool db::checkSqliteAvailable() {
    sqlite3* handle = nullptr;
    const int rc = sqlite3_open(":memory:", &handle);
    const bool ok = (rc == SQLITE_OK && handle != nullptr);
    if (handle) sqlite3_close(handle);
    return ok;
}

// Create (or open) a database file at 'path'; does not create schema.
// Returns true on success; false on failure. On failure, outError may be:
// "unable to open database file", "out of memory", "database is locked", etc.
bool db::createDatabase(const std::string& path, std::string* outError) {
    sqlite3* handle = nullptr;
    // Explicitly open for read/write, creating the file if it doesn't exist.
    constexpr int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    const int rc = sqlite3_open_v2(path.c_str(), &handle, flags, nullptr);
    const bool ok = (rc == SQLITE_OK && handle != nullptr);
    if (!ok && outError) {
        if (handle) {
            *outError = sqlite3_errmsg(handle);
        } else {
            *outError = sqlite3_errstr(rc);
        }
    }
    if (handle) sqlite3_close(handle);
    return ok;
}

// Helper: open DB, run SQL, set outError on failure, close DB.
bool db::execSql(const std::string& path, const char* sql, std::string* outError) {
    sqlite3* handle = nullptr;
    constexpr int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    if (const int rc_open = sqlite3_open_v2(path.c_str(), &handle, flags, nullptr); rc_open != SQLITE_OK || handle == nullptr) {
        if (outError) *outError = sqlite3_errstr(rc_open);
        if (handle) sqlite3_close(handle);
        return false;
    }

    char* errMsg = nullptr;
    if (const int rc_exec = sqlite3_exec(handle, sql, nullptr, nullptr, &errMsg); rc_exec != SQLITE_OK) {
        if (outError) {
            if (errMsg) {
                *outError = errMsg;
            } else {
                *outError = sqlite3_errstr(rc_exec);
            }
        }
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(handle);
        return false;
    }

    sqlite3_close(handle);
    return true;
}

/*
 * Create a single table by executing the provided CREATE TABLE SQL on the database at 'path'.
 * Args (examples):
 *  - path:
 *      "data/app.db"            // relative path (parent dir must exist)
 *      "/var/www/app/app.db"    // absolute path
 *
 *  - createTableSql:
 *      "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT);"
 *      "CREATE TABLE products (sku TEXT PRIMARY KEY, name TEXT, price_cents INTEGER);"
 *      (use IF NOT EXISTS for idempotence; multiple statements allowed separated by ';')
 *   - outError:
 *      nullptr                 // caller ignores error message
 *      &err (std::string)      // on failure receives messages like:
 *                              // "syntax error", "unable to open database file",
 *                              // "table users already exists", "database is locked"
 * Returns: true on success, false on failure.
 * Possible outputs (examples):
 *  - true
 *  - false, outError = "empty SQL"
 *  - false, outError = "syntax error"
 *  - false, outError = "unable to open database file"
*/
bool db::createTable(const std::string& path, const std::string& createTableSql, std::string* outError) {
    if (createTableSql.empty()) {
        if (outError) *outError = "empty SQL";
        return false;
    }
    return execSql(path, createTableSql.c_str(), outError);
}
