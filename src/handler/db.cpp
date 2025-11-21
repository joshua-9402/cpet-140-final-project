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
 *
 * Database:
 * For Payroll System:
 * |----------|----------------|-----------|-------------------------|--------------------------|
 * | ID       | Name           | Position  | Salary (hour per PHP)   | Hours Worked (per Month) |
 * |----------|----------------|-----------|-------------------------|--------------------------|
 * | 000001   | Juan Dela Cruz | Secretary | 70.00                   | 192  (24 * 8)            |
 * |----------|----------------|-----------|-------------------------|--------------------------|
 * | 000002   | Maria Santos   | Manager   | 100.00                  | 208  (26 * 8)            |
 * |----------|----------------|-----------|-------------------------|--------------------------|
 * | 000003   | Pedro Reyes    | Engineer  | 200.00                  | 176  (22 * 8)            |
 * |----------|----------------|-----------|-------------------------|--------------------------|
 * | 000004   | Ana Lopez      | Technician| 80.00                   | 200  (25 * 8)            |
 * |----------|----------------|-----------|-------------------------|--------------------------|
 * | 000005   | Luis Garcia    | Laborer   | 30.00                   | 184  (23 * 8)            |
 * |----------|----------------|-----------|-------------------------|--------------------------|
 *
 * For Tracker/Monitoring System:
 * - status can be "Active", "Completed", "On-Hold", or "In Progress".
 * - For date, use ISO 8601 format (year-month-day) (e.g., "2023-10-05").
 * |------------|--------------------------------------------|-------------|--------------------|----------------------|
 * | Project ID | Project Name                               | Status      | Project Start Date | Notes                |
 * |------------|--------------------------------------------|-------------|--------------------|----------------------|
 * | PRJ-00001  | BatStateU Aboitiz LIMA Campus Construction | In Progress | 2025-01-15         | Under Construction   |
 * |------------|--------------------------------------------|-------------|--------------------|----------------------|
 *
 * For Tracker/Monitoring System Building Materials (Per Project) (materials/${PROJECT_ID}-Materials.db, e.g., materials/PRJ-0001-Materials.db):
 * |------------------|------------------|-------------------------|------------------|
 * | Material ID      | Material Name    | Quantity                | Unit Price (PHP) |
 * |                  |                  | (in unit appropriate    |                  |
 * |                  |                  | with the material)      |                  |
 * |------------------|------------------|-------------------------|------------------|
 * | MAT-0001         | Cement           | 100 (bags)              | 250.00           |
 * |------------------|------------------|-------------------------|------------------|
 * | MAT-0002         | Steel Rebars     | 500 (kilograms)         | 75.00            |
 * |------------------|------------------|-------------------------|------------------|
 * | MAT-0003         | Gravel           | 2 (cubic meters)        | 1500.00          |
 * |------------------|------------------|-------------------------|------------------|
 *
 */


#include "db.h"
#include "../config/config.h"
#include <sqlite3.h>
#include <string>
#include <fstream>
#include <filesystem>


bool db::createDatabase(const std::string& p_dbName) {
    sqlite3* dbPtr; // Pointer to the SQLite database connection

    int database = sqlite3_open(p_dbName.c_str(), &dbPtr);

    if (database) { if (dbPtr) sqlite3_close(dbPtr); return false; }

    if (p_dbName == appConfig::g_dbNamePayroll) {
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS Employees ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "Name TEXT NOT NULL,"
            "Position TEXT NOT NULL,"
            "Salary REAL NOT NULL,"
            "HoursWorked REAL NOT NULL"
            ");";
        database = sqlite3_exec(dbPtr, databaseTable.c_str(), nullptr, nullptr, nullptr);
    }
    else if (p_dbName == appConfig::g_dbNameTracker) {
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS Projects ("
            "ProjectID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "ProjectName TEXT NOT NULL,"
            "Status TEXT NOT NULL,"
            "StartDate TEXT NOT NULL,"
            "Notes TEXT"
            ");";
        database = sqlite3_exec(dbPtr, databaseTable.c_str(), nullptr, nullptr, nullptr);
    }
    else if (p_dbName == appConfig::g_dbNameUsers) {
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS Projects ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT,"
            "passwords TEXT NOT NULL,"
            "Status TEXT NOT NULL,"
            "dateCreated TEXT NOT NULL,"
            "fullName TEXT NOT NULL,"
            ");";
    }
    else {
        sqlite3_close(dbPtr);
        return false; // Unknown database name
    }

    if (database != SQLITE_OK) { if (dbPtr) sqlite3_close(dbPtr); return false; }

    // Close the database connection
    sqlite3_close(dbPtr);

    return true;
}


bool db::searchDatabase(const std::string& p_filename) {
    // Prefer std::filesystem when available
    try {
        if (std::filesystem::exists(p_filename)) return true;
        if (std::filesystem::exists(p_filename + ".db")) return true;
    } catch (...) {
        // fall back
    }

    // Fallback: try opening the file
    if (std::ifstream f(p_filename); f.is_open()) { f.close(); return true; }
    if (std::ifstream f2(p_filename + ".txt"); f2.is_open()) { f2.close(); return true; }
    return false;
}


bool db::openDatabase(const std::string& p_dbName) {
    sqlite3* dbPtr = nullptr; // Database connection object

    if (const int database = sqlite3_open(p_dbName.c_str(), &dbPtr); database == SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return true;
    }
    return false;
}


bool db::closeDatabase(const std::string& p_dbName) {
    sqlite3* dbPtr = nullptr; // Pointer to the SQLite database connection

    int rc = sqlite3_open(p_dbName.c_str(), &dbPtr);
    if (rc != SQLITE_OK) { if (dbPtr) { sqlite3_close(dbPtr); } return false; }

    rc = sqlite3_close(dbPtr);
    return (rc == SQLITE_OK);
}


bool db::appendToDatabase(const std::string& p_dbName, std::string& p_data) {
    // Simple wrapper - execute given SQL
    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return false;
    }
    char* err = nullptr;
    int rc = sqlite3_exec(dbPtr, p_data.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    return rc == SQLITE_OK;
}

bool db::isSQLiteAvailable() {
    // Check if SQLite library is available by verifying the version
    const char* version = sqlite3_libversion();
    if (version == nullptr || version[0] == '\0') {
        return false;
    }

    // Alternatively, try to open and close an in-memory database
    sqlite3* testDb = nullptr;
    int rc = sqlite3_open(":memory:", &testDb);
    if (rc == SQLITE_OK && testDb != nullptr) {
        sqlite3_close(testDb);
        return true;
    }

    if (testDb) sqlite3_close(testDb);
    return false;
}
