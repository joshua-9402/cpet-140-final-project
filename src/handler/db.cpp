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
 * This will be the structure of the database files used in the application.

    root/
      └── data/
          ├── payroll/
          │   ├── 2025/
          │   │   └── 11/17-22.db
          │   └── base_payroll.db
          └── projects/
              ├── expense/
              │   ├── PRJ-0001.db
              │   └── PRJ-0002.db
              ├── materials.db
              ├── materials-custom.db
              └── base_projects.db

 * For Payroll System:
 * This is the template and for every month, there will be new folder and every folder contains four database files (base_payroll.db)
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 * | Employee ID | Name           | Position  | Location | Salary (hour per PHP)   | Hours Worked (per week)  | Advance        |
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 * | 000001      | Juan Dela Cruz | Secretary | Manila   | 70.00                   | 192  (24 * 8)            | 0.00           |
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 * | 000002      | Maria Santos   | Manager   | Cavite   | 100.00                  | 208  (26 * 8)            | 5000.00        |
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 * | 000003      | Pedro Reyes    | Engineer  | Iloilo   | 200.00                  | 176  (22 * 8)            | 1500.00        |
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 * | 000004      | Ana Lopez      | Technician| Baguio   | 80.00                   | 200  (25 * 8)            | 2500.00        |
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 * | 000005      | Luis Garcia    | Laborer   | Davao    | 30.00                   | 184  (23 * 8)            | 0.00           |
 * |-------------|----------------|-----------|----------|-------------------------|--------------------------|----------------|
 *
 * For tracking employees' worked hours for a week:
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 * | Employee ID      | Mon | Tue | Wed | Thu | Fri | Sat | Sun |
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 * | 000001           | 08  | 08  | 08  | 08  | 08  | 00  | 00  |
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 * | 000002           | 08  | 08  | 07  | 08  | 06  | 04  | 00  |
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 * | 000003           | 08  | 06  | 08  | 08  | 05  | 00  | 00  |
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 * | 000004           | 08  | 07  | 08  | 08  | 08  | 08  | 00  |
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 * | 000005           | 06  | 06  | 06  | 06  | 06  | 00  | 00  |
 * |------------------|-----|-----|-----|-----|-----|-----|-----|
 *
 * Questions:
 *  - For the computation of the salary, is it just hourly rate multiplied by hours worked?
 *
 *
 * For Tracker/Monitoring System:
 * - status can be "Active", "Completed", "On-Hold", or "In Progress".
 * - For date, use ISO 8601 format (year-month-day) (e.g., "2023-10-05").
 * - this is the main database file for tracking projects (projects/base_projects.db)
 * |------------|--------------------------------------------|-------------|--------------------|-----------------------------------|
 * | Project ID | Project Name                               | Status      | Project Start Date | Notes                             |
 * |------------|--------------------------------------------|-------------|--------------------|-----------------------------------|
 * | PRJ-00001  | BatStateU Aboitiz LIMA Campus              | In Progress | 2025-01-15         | Under Construction at LIMA Estate |
 * |------------|--------------------------------------------|-------------|--------------------|-----------------------------------|
 * | PRJ-00002  | BatStateU STEER Hub                        | Completed   | 2024-11-01         | BatStateU Gov. Pablo Borbon II    |
 * |------------|--------------------------------------------|-------------|--------------------|-----------------------------------|
 *
 *
 * For Tracker/Monitoring System Building Materials (Per Project) (expense/${PROJECT_ID}.db, e.g., expense/PRJ-0001.db):
 *  - MAT-{number} - a predefined material ID.
 *  - MAT-CS-{number} - custom/special order material ID.
 *
 * |------------------|-----------------------|-------------------------|------------------|
 * | Material ID      | Material Name         | Quantity                | Unit Price (PHP) |
 * |                  |                       | (in unit appropriate    |                  |
 * |                  |                       | with the material)      |                  |
 * |------------------|-----------------------|-------------------------|------------------|
 * | MAT-0000         | Bidding Documents     | 0 (units)               | 0.00             |
 * |------------------|-----------------------|-------------------------|------------------|
 * | MAT-0001         | Cement                | 100 (bags)              | 250.00           |
 * |------------------|-----------------------|-------------------------|------------------|
 * | MAT-0002         | Steel Rebars          | 500 (kilograms)         | 75.00            |
 * |------------------|-----------------------|-------------------------|------------------|
 * | MAT-0003         | Gravel                | 2 (cubic meters)        | 1500.00          |
 * |------------------|-----------------------|-------------------------|------------------|
 * | MAT-CS-0001      | Drill Bit Set- Shopee | 3 (cubic meters)        | 1200.00          |
 * |------------------|-----------------------|-------------------------|------------------|
 *
 */


#include "db.h"
#include "../config/config.h"
#include <sqlite3.h>
#include <string>
#include <fstream>


bool db::createDatabase(const std::string& p_dbName) {
    sqlite3* dbPtr; // Pointer to the SQLite database connection

    int database = sqlite3_open(p_dbName.c_str(), &dbPtr);

    if (database) { if (dbPtr) sqlite3_close(dbPtr); return false; }

    if (p_dbName == appConfig::g_dbNamePayroll) {
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS EMPLOYEES ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "Name TEXT NOT NULL,"
            "Position TEXT NOT NULL,"
            "Location TEXT NOT NULL,"
            "Salary REAL NOT NULL,"
            "HoursWork REAL NOT NULL"
            ",Advance REAL NOT NULL"
            ");";
        database = sqlite3_exec(dbPtr, databaseTable.c_str(), nullptr, nullptr, nullptr);
    }
    else if (p_dbName == appConfig::g_dbNameProject) {
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS PROJECT_LIST ("
            "ProjectID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "ProjectName TEXT NOT NULL,"
            "Status TEXT NOT NULL,"
            "StartDate TEXT NOT NULL,"
            ");";
        database = sqlite3_exec(dbPtr, databaseTable.c_str(), nullptr, nullptr, nullptr);
    }
    else if (p_dbName == "PRJ-001" " .db") {
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS MATERIALS ("
            "Material-ID TEXT NOT NULL,"
            "Material-Name TEXT NOT NULL,"
            "Quantity TEXT NOT NULL,"
            "PRICE-PER-UNIT TEXT NOT NULL,"
            ");";
        database = sqlite3_exec(dbPtr, databaseTable.c_str(), nullptr, nullptr, nullptr);
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


bool db::openDatabase(const std::string& p_dbName) {
    sqlite3* dbPtr = nullptr; // Database connection object

    if (const int database = sqlite3_open(p_dbName.c_str(), &dbPtr); database == SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return true;
    }
    return false;
}


bool db::appendDatabase(const std::string& p_dbName, const std::string& p_data) {
    // Simple wrapper - execute given SQL
    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return false;
    }
    char* err = nullptr;
    const int rc = sqlite3_exec(dbPtr, p_data.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    return rc == SQLITE_OK;
}

bool db::isSQLiteAvailable() {
    // Check if SQLite library is available by verifying the version
    if (const char* version = sqlite3_libversion(); version == nullptr || version[0] == '\0') {
        return false;
    }

    // Alternatively, try to open and close an in-memory database
    sqlite3* testDb = nullptr;
    if (const int rc = sqlite3_open(":memory:", &testDb); rc == SQLITE_OK && testDb != nullptr) {
        sqlite3_close(testDb);
        return true;
    }

    if (testDb) sqlite3_close(testDb);
    return false;
}
