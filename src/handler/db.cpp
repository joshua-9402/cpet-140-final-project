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
 * | MAT-0003         | Gravel           | 2 (cubic meters)         | 1500.00         |
 * |------------------|------------------|-------------------------|------------------|
 *
 */


#include "db.h"
#include "../config/app_config.h"
#include <sqlite3.h>
#include <string>
#include <fstream> // Required for file operations


bool createFileText(const std::string& p_filename) {
    if (const std::ofstream createdFile(p_filename); createdFile.is_open()) {return true;}
    return false;
}


std::string readFileText(const std::string& p_filename, const int p_lineFileText) {

    // Try the exact filename first, then fallback to filename + ".txt"
    std::ifstream inputFile(p_filename);
    if (!inputFile.is_open()) {inputFile.clear(); inputFile.open(p_filename + ".txt");}

    if (!inputFile.is_open()) {return "";} // not found / error

    std::string line;
    int currentLineNumber = 0;
    while (std::getline(inputFile, line)) {++currentLineNumber; if (currentLineNumber == p_lineFileText) {return line;}}
    return ""; // line not found
}


bool appendFileText(const std::string& p_filename, const std::string& p_newText, const bool addNewline = true) {
    std::ofstream outputFile(p_filename, std::ios::app);

    if (!outputFile.is_open()) return false;

    outputFile << p_newText;
    if (addNewline) outputFile << '\n';

    // flush and check for errors
    outputFile.flush();
    return !outputFile.fail();
}


bool createDatabase(const std::string& p_dbName) {
    sqlite3* db; // Pointer to the SQLite database connection

    // Return code for SQLite operations
    // Open/create the database file
    // If "example.db" doesn't exist, it will be created.
    int database = sqlite3_open(p_dbName.c_str(), &db);


    if (database) {return false;}

    if (p_dbName == g_dbNamePayroll) {
        // SQL statement to create the Employees table for Payroll System
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS Employees ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "Name TEXT NOT NULL,"
            "Position TEXT NOT NULL,"
            "Salary REAL NOT NULL,"
            "HoursWorked REAL NOT NULL"
            ");";
        // Execute the SQL statement
        database = sqlite3_exec(db, databaseTable.c_str(), nullptr, nullptr, nullptr);
    }
    else if (p_dbName == g_dbNameTracker) {
        // SQL statement to create the Projects table for Tracker/Monitoring System
        const std::string databaseTable =
            "CREATE TABLE IF NOT EXISTS Projects ("
            "ProjectID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "ProjectName TEXT NOT NULL,"
            "Status TEXT NOT NULL,"
            "StartDate TEXT NOT NULL,"
            "Notes TEXT"
            ");";
        // Execute the SQL statement
        database = sqlite3_exec(db, databaseTable.c_str(), nullptr, nullptr, nullptr);
    }
    else {
        sqlite3_close(db);
        return false; // Unknown database name
    }

    if (database != SQLITE_OK) {return false;}

    // Close the database connection
    sqlite3_close(db);

    return true;
}


bool openDatabase(const std::string& p_dbName) {
    sqlite3* db; // Database connection object

    if (const int database = sqlite3_open(p_dbName.c_str(), &db); database == SQLITE_OK) {return true;}
    return false;
}


bool closeDatabase(const std::string& p_dbName) {
    sqlite3* db; // Pointer to the SQLite database connection

    // Open the database to obtain a valid handle before closing.
    int rc = sqlite3_open(p_dbName.c_str(), &db);
    if (rc != SQLITE_OK) { if (db) { sqlite3_close(db); } return false;}

    // Close the database and report status.
    rc = sqlite3_close(db);
    return (rc == SQLITE_OK);
}
