//
// Created by Josh Literal on 11/1/25.
//

#ifndef CPET_140_FINAL_PROJECT_DB_H
#define CPET_140_FINAL_PROJECT_DB_H

#include <string>

/**
 * SQLite utility helpers.
 * - Availability check (no side effects)
 * - Lightweight database file creation (no schema creation)
 */
class db {
public:
    /**
     * Check if the SQLite library is available and functional.
     * Opens and closes an in-memory database; no files are created.
     *
     * Returns
     * - true  -> SQLite is available and can open an in-memory DB.
     * - false -> SQLite is not available or failed to open.
     *
     * Possible outputs (examples)
     * - true
     * - false
     */
    static bool checkSqliteAvailable();

    /**
     * Create (or open if it already exists) a SQLite database file at the given path.
     * Does not create tables/schema; only ensures the file can be opened by SQLite.
     * The parent directory must already exist.
     *
     * Behavior
     * - If a database file already exists at 'path' the function will open it.
     * - If no file exists the function will create a new database file.
     *
     * Returns
     * - true  -> File was opened or created successfully.
     * - false -> Failed to open/create the database file.
     *
     * Possible outError values (examples)
     * - "unable to open database file"
     * - "out of memory"
     * - "database is locked"
     * - "not a database"
     *
     * @param path Filesystem path to the .db file.
     * @param outError Optional: receives a human-readable error on failure.
     */
    static bool createDatabase(const std::string& path, std::string* outError = nullptr);

    /**
     * Create a single table by executing the provided CREATE TABLE SQL on the database at 'path'.
     * - Opens (or creates) the DB file, executes the SQL, and closes the DB.
     * - The provided SQL should be a valid SQL statement (commonly a single
     *   "CREATE TABLE ..." statement). Use "IF NOT EXISTS" if you want the call
     *   to be idempotent.
     *
     * Parameters
     * @param path
     *   Filesystem path to the SQLite database file. If the file does not exist it
     *   will be created (parent directory must exist). Example: "data/app.db"
     *
     * @param createTableSql
     *   The SQL statement to execute. Typically, a single CREATE TABLE statement,
     *   e.g.:
     *     "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT);"
     *   The string may contain multiple statements separated by semicolons if needed.
     *
     * @param outError
     *   Optional output parameter. On failure the function writes a human-readable
     *   SQLite error message into outError. If nullptr is passed no message is returned.
     *   Examples:
     *     - "table users already exists" (if not using IF NOT EXISTS)
     *     - "syntax error"
     *     - "unable to open database file"
     *
     * Returns
     * - true  -> The statement executed successfully.
     * - false -> Execution failed; outError (if provided) contains error details.
     */
    static bool createTable(const std::string& path, const std::string& createTableSql, std::string* outError = nullptr);

    static bool execSql(const std::string& path, const char* sql, std::string* outError);
};


#endif //CPET_140_FINAL_PROJECT_DB_H