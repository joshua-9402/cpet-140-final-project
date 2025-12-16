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
          │   │   └── 11/17-22/2025.db
          │   └── base_payroll.db
          └── projects/
              ├── expense/
              │   ├── PRJ-0001.db
              │   └── PRJ-0002.db
              └── base_projects.db

 * For Payroll System:
 * This is the template and for every month, there will be new folder and every folder contains four database files (base_payroll.db)
 * |-------------|----------------|-----------|---------------|-------------------------|--------------------------|----------------|
 * | EMPLOYEE_ID | NAME           | Position  | SITE_LOCATION | SALARY   | HOURS_WORKED  | ADVANCE       |
 * |-------------|----------------|-----------|---------------|-------------------------|--------------------------|----------------|
 * | 000001      | Juan Dela Cruz | Secretary | Manila   | 70.00                   | 192            | 0.00           |
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
 * | EMPLOYEE_ID      | Mon | Tue | Wed | Thu | Fri | Sat | Sun |
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
 * For Tracker/Monitoring System Building Materials (Per Project) (expense/${PROJECT_ID}.db, e.g., expense/PRJ-00001.db):
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
#include "system.h"
#include <sqlite3.h>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>


bool db::isSQLiteAvailable() {
    // Check if the SQLite library is available by verifying the version
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


bool db::createDatabase(const std::string& p_dbName) {
    sqlite3* dbPointer = nullptr;

    // Open (or create) a database file
    if (const int returnCode = sqlite3_open_v2(p_dbName.c_str(), &dbPointer,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr); returnCode != SQLITE_OK) {
        if (dbPointer) sqlite3_close(dbPointer);
        system::logMessage(system::messageClassification::ERROR, "DB CREATE failed db=" + p_dbName);
        return false;
    }

    std::vector<std::string> tablesSql;

    // Normalize to filename only so callers can pass full paths
    std::string filename = p_dbName;
    if (const size_t pos = filename.find_last_of("/\\"); pos != std::string::npos) {
        filename = filename.substr(pos + 1);
    }


    // Use column names that match the rest of the code (insert/update)
    if (filename == appConfig::g_dbNamePayroll || p_dbName == appConfig::g_dbNamePayroll) {
        tablesSql.push_back(
            "CREATE TABLE IF NOT EXISTS EMPLOYEES ("
            "EMPLOYEE_ID INTEGER PRIMARY KEY AUTOINCREMENT,"
            "NAME TEXT NOT NULL,"
            "POSITION TEXT NOT NULL,"
            "SITE_LOCATION TEXT NOT NULL,"
            "SALARY REAL NOT NULL,"
            "HOURS_WORK REAL NOT NULL,"
            "ADVANCE REAL NOT NULL"
            ");"
        );
        // Integrate weekly attendance into base_payroll.db as a central ledger
        tablesSql.push_back(
            "CREATE TABLE IF NOT EXISTS WEEKLY_ATTENDANCE ("
            "EMPLOYEE_ID TEXT NOT NULL,"
            "WEEK_START TEXT NOT NULL,"
            "SUN REAL NOT NULL,"
            "MON REAL NOT NULL,"
            "TUE REAL NOT NULL,"
            "WED REAL NOT NULL,"
            "THU REAL NOT NULL,"
            "FRI REAL NOT NULL,"
            "SAT REAL NOT NULL,"
            "PRIMARY KEY (EMPLOYEE_ID, WEEK_START)"
            ");"
        );
    } else if (filename == appConfig::g_dbNameProject || p_dbName == appConfig::g_dbNameProject) {
        tablesSql.push_back(
            "CREATE TABLE IF NOT EXISTS PROJECT_LIST ("
            "PROJECT_ID TEXT PRIMARY KEY,"
            "PROJECT_NAME TEXT NOT NULL,"
            "STATUS TEXT NOT NULL,"
            "START_DATE TEXT NOT NULL,"
            "NOTE TEXT"
            ");"
        );
    } else if (
        filename.find("PRJ") != std::string::npos &&
        filename.find(".db") != std::string::npos){
        tablesSql.push_back(
            "CREATE TABLE IF NOT EXISTS MATERIALS ("
            "MATERIAL_ID TEXT PRIMARY KEY,"
            "MATERIAL_NAME TEXT NOT NULL,"
            "QUANTITY REAL NOT NULL,"
            "UNIT_PRICE REAL NOT NULL"
            ");"
        );
    } else if (
        // Weekly attendance database detection (format: MM-DD-DD.db or MM-DD-MM-DD.db)
        // Must have 2 or 3 hyphens and be short filename
        (std::count(filename.begin(), filename.end(), '-') == 2 ||
         std::count(filename.begin(), filename.end(), '-') == 3) &&
        filename.find(".db") != std::string::npos &&
        filename.length() < 20 &&
        filename.find("PRJ") == std::string::npos) {
        // Attendance database for weekly records
        tablesSql.push_back(
            "CREATE TABLE IF NOT EXISTS WEEKLY_ATTENDANCE ("
            "EMPLOYEE_ID TEXT NOT NULL,"
            "WEEK_START TEXT NOT NULL,"
            "SUN REAL NOT NULL,"
            "MON REAL NOT NULL,"
            "TUE REAL NOT NULL,"
            "WED REAL NOT NULL,"
            "THU REAL NOT NULL,"
            "FRI REAL NOT NULL,"
            "SAT REAL NOT NULL,"
            "PRIMARY KEY (EMPLOYEE_ID, WEEK_START)"
            ");"
        );
    } else if (
        filename.find("PRJ-") == std::string::npos &&
        filename.find("20") != std::string::npos &&
        filename.find(".db") != std::string::npos) {
        tablesSql.push_back(
            "CREATE TABLE IF NOT EXISTS TIMESHEET ("
            "EMPLOYEE_ID INTEGER PRIMARY KEY,"
            "MON REAL NOT NULL,"
            "TUE REAL NOT NULL,"
            "WED REAL NOT NULL,"
            "THU REAL NOT NULL,"
            "FRI REAL NOT NULL,"
            "SAT REAL NOT NULL,"
            "SUN REAL NOT NULL"
            ");"
        );
    } else {
        sqlite3_close(dbPointer);
        return false;
    }

    // Execute all queued table creation statements
    for (const auto& sqlStmt : tablesSql) {
        char* err = nullptr;
        if (const int execRc = sqlite3_exec(dbPointer, sqlStmt.c_str(), nullptr, nullptr, &err); execRc != SQLITE_OK) {
            if (err) sqlite3_free(err);
            sqlite3_close(dbPointer);
            system::logMessage(system::messageClassification::ERROR, "DB CREATE failed during schema init db=" + p_dbName);
            return false;
        }
    }

    sqlite3_close(dbPointer);
    // Avoid logging data content; only operation and target DB
    system::logMessage(system::messageClassification::INFO, "DB CREATE ok db=" + p_dbName);
    return true;
}


bool db::openDatabase(const std::string& p_dbName) {
    sqlite3* dbPtr = nullptr;

    if (const int database = sqlite3_open(p_dbName.c_str(), &dbPtr); database == SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::INFO, "DB OPEN ok db=" + p_dbName);
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB OPEN failed db=" + p_dbName);
    return false;
}


bool db::closeDatabase() {
    // If a persistent sqlite3\* connection member (e.g., m_db) is added later, close it here:
    //     if (m_db) { sqlite3_close(m_db); m_db = nullptr; }
    // As a best-effort cleanup, shut down the SQLite library to free resources.
    const int rc = sqlite3_shutdown();
    if (rc == SQLITE_OK) {
        system::logMessage(system::messageClassification::INFO, "DB CLOSE ok");
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB CLOSE failed");
    return false;
}


// p_data should contain comma-separated values matching the column order (e.g., "'John Doe', 'Manager', 'Manila', 100.0, 160.0, 0.0").
bool db::appendDatabase(const std::string& p_dbName, const std::string& p_data) {
    // Check if database exists and has content; if not or empty, create it with proper schema
    std::ifstream dbCheck(p_dbName, std::ios::binary | std::ios::ate);
    bool needsCreation = false;

    if (!dbCheck.good()) {
        needsCreation = true;
    } else {
        // Check file size - if 0 bytes, we need to create the schema
        const std::streamsize size = dbCheck.tellg();
        if (size == 0) {
            needsCreation = true;
        }
    }
    dbCheck.close();

    if (needsCreation) {
        if (!createDatabase(p_dbName)) {
            system::logMessage(system::messageClassification::ERROR, "DB INSERT failed (create) db=" + p_dbName);
            return false;
        }
    }

    sqlite3* dbPointer = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPointer) != SQLITE_OK) {
        if (dbPointer) sqlite3_close(dbPointer);
        system::logMessage(system::messageClassification::ERROR, "DB INSERT failed (open) db=" + p_dbName);
        return false;
    }

    // Normalize to filename only for checking
    std::string filename = p_dbName;
    if (const size_t pos = filename.find_last_of("/\\"); pos != std::string::npos) {
        filename = filename.substr(pos + 1);
    }

    std::string sqlite;
    if (p_dbName == appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll) {
        sqlite = "INSERT INTO EMPLOYEES (NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE) VALUES (" + p_data + ");";
    } else if (p_dbName == appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject) {
        // ...existing code...
        sqlite = "INSERT INTO PROJECT_LIST (PROJECT_ID, PROJECT_NAME, STATUS, START_DATE, NOTE) VALUES (" + p_data + ");";

        // Extract PROJECT_ID from p_data to create corresponding expense database
        // ...existing code...
        size_t firstQuote = p_data.find('\'');
        size_t secondQuote = p_data.find('\'', firstQuote + 1);
        if (firstQuote != std::string::npos && secondQuote != std::string::npos) {
            std::string projectId = p_data.substr(firstQuote + 1, secondQuote - firstQuote - 1);

            // Create expense database for this project: expense/PRJ-00001.db
            std::string expenseDbPath = appConfig::g_dataDirectory +
                                       appConfig::g_projectDirectory +
                                       appConfig::g_projectExpenseDirectory +
                                       projectId + ".db";

            // Create the expense database with MATERIALS table
            createDatabase(expenseDbPath);
        }
    } else if (filename.find("PRJ-") != std::string::npos && filename.find(".db") != std::string::npos) {
        // Material database (expense/PRJ-xxxxx.db)
        sqlite = "INSERT INTO MATERIALS (MATERIAL_ID, MATERIAL_NAME, QUANTITY, UNIT_PRICE) VALUES (" + p_data + ");";
    } else if (
        // SIMPLIFIED: Attendance database - has hyphen, not PRJ, has .db
        filename.find("-") != std::string::npos &&
        filename.find(".db") != std::string::npos &&
        filename.find("PRJ") == std::string::npos) {
        // Attendance database
        sqlite = "INSERT INTO WEEKLY_ATTENDANCE (EMPLOYEE_ID, WEEK_START, SUN, MON, TUE, WED, THU, FRI, SAT) VALUES (" + p_data + ");";
    } else {
        if (dbPointer) sqlite3_close(dbPointer);
        return false;
    }

    char* err = nullptr;
    const int rc = sqlite3_exec(dbPointer, sqlite.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPointer);
    if (rc == SQLITE_OK) {
        // Identify entity by db filename without exposing values
        std::string filename = p_dbName;
        if (const size_t pos = filename.find_last_of("/\\"); pos != std::string::npos) filename = filename.substr(pos + 1);
        std::string entity = (filename == appConfig::g_dbNamePayroll) ? "EMPLOYEE"
                            : (filename == appConfig::g_dbNameProject) ? "PROJECT"
                            : (filename.find("PRJ-") != std::string::npos ? "MATERIAL" : "ATTENDANCE");
        system::logMessage(system::messageClassification::INFO, "DB INSERT ok db=" + p_dbName + " entity=" + entity);
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB INSERT failed db=" + p_dbName);
    return false;
}


// p_id is the ID/ProjectID to update; p_data contains SET clause values (e.g., "Name='John Smith', Position='Senior Manager', Salary=150.0")
bool db::updateDatabase(const std::string& p_dbName, const std::string& p_id, const std::string& p_data) {
    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB UPDATE failed (open) db=" + p_dbName);
        return false;
    }

    std::string sql;

    // Normalize to filename only for checking
    std::string filename = p_dbName;
    if (const size_t pos = filename.find_last_of("/\\"); pos != std::string::npos) {
        filename = filename.substr(pos + 1);
    }

    if (p_dbName == appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll) {
        sql = "UPDATE EMPLOYEES SET " + p_data + " WHERE EMPLOYEE_ID = " + p_id + ";";
    } else if (p_dbName == appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject) {
        sql = "UPDATE PROJECT_LIST SET " + p_data + " WHERE PROJECT_ID = '" + p_id + "';";
    } else if (filename.find("PRJ-") != std::string::npos && filename.find(".db") != std::string::npos) {
        // Material database (expense/PRJ-xxxxx.db)
        sql = "UPDATE MATERIALS SET " + p_data + " WHERE MATERIAL_ID = '" + p_id + "';";
    } else if (
        // SIMPLIFIED: Attendance database - has hyphen, not PRJ, has .db
        filename.find("-") != std::string::npos &&
        filename.find(".db") != std::string::npos &&
        filename.find("PRJ") == std::string::npos) {
        // Attendance database - update by EMPLOYEE_ID
        sql = "UPDATE WEEKLY_ATTENDANCE SET " + p_data + " WHERE EMPLOYEE_ID = '" + p_id + "';";
    } else {
        sqlite3_close(dbPtr);
        return false;
    }

    char* err = nullptr;
    const int rc = sqlite3_exec(dbPtr, sql.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    if (rc == SQLITE_OK) {
        system::logMessage(system::messageClassification::INFO, "DB UPDATE ok db=" + p_dbName + " key=provided");
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB UPDATE failed db=" + p_dbName);
    return false;
}

// Delete a row by employee ID or project ID
bool db::deleteRow(const std::string& p_dbName, const std::string& p_id) {
    // Validate input is not empty
    if (p_id.empty()) {
        system::logMessage(system::messageClassification::WARNING, "DB DELETE no-op (empty key) db=" + p_dbName);
        return false;
    }

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB DELETE failed (open) db=" + p_dbName);
        return false;
    }

    // Determine table and primary key column
    std::string filename = p_dbName;
    if (const size_t pos = filename.find_last_of("/\\"); pos != std::string::npos) {
        filename = filename.substr(pos + 1);
    }

    std::string sql;
    bool isNumericId = std::all_of(p_id.begin(), p_id.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });

    if (filename == appConfig::g_dbNamePayroll) {
        if (!isNumericId) {
            sqlite3_close(dbPtr);
            return false;
        }
        sql = "DELETE FROM EMPLOYEES WHERE EMPLOYEE_ID = ?;";
    } else if (filename == appConfig::g_dbNameProject) {
        // Project ID is text (PRJ-xxxxx format)
        sql = "DELETE FROM PROJECT_LIST WHERE PROJECT_ID = ?;";
    } else if (filename.find("PRJ-") != std::string::npos && filename.find(".db") != std::string::npos) {
        // Material database (expense/PRJ-xxxxx.db)
        sql = "DELETE FROM MATERIALS WHERE MATERIAL_ID = ?;";
    } else if (
        // SIMPLIFIED: Attendance database - has hyphen, not PRJ, has .db
        filename.find("-") != std::string::npos &&
        filename.find(".db") != std::string::npos &&
        filename.find("PRJ") == std::string::npos) {
        // Attendance database - delete by EMPLOYEE_ID
        sql = "DELETE FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID = ?;";
    } else {
        sqlite3_close(dbPtr);
        return false;
    }

    // Prepare and execute a statement
    sqlite3_stmt* stmt = nullptr;
    bool success = false;

    if (sqlite3_prepare_v2(dbPtr, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (filename == appConfig::g_dbNamePayroll) {
            sqlite3_bind_int64(stmt, 1, std::stoll(p_id));
        } else {
            sqlite3_bind_text(stmt, 1, p_id.c_str(), -1, SQLITE_STATIC);
        }
        success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
    }

    sqlite3_close(dbPtr);
    if (success) {
        system::logMessage(system::messageClassification::INFO, "DB DELETE ok db=" + p_dbName + " key=provided");
        return true;
    }
    system::logMessage(system::messageClassification::WARNING, "DB DELETE no-op or failed db=" + p_dbName);
    return false;
}

// Check if there are gaps in employee IDs that need rearranging
bool db::checkEmployeeChanges() {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(dbPath.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return false;
    }

    // Query to check if there are gaps in employee IDs
    sqlite3_stmt* stmt = nullptr;
    bool hasGaps = false;

    if (const auto sql = "SELECT EMPLOYEE_ID FROM EMPLOYEES ORDER BY EMPLOYEE_ID;"; sqlite3_prepare_v2(dbPtr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        int expectedId = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (const int currentId = sqlite3_column_int(stmt, 0); currentId != expectedId) {
                hasGaps = true;
                break;
            }
            expectedId++;
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(dbPtr);
    return hasGaps;
}

// Rearrange employee IDs to eliminate gaps (1, 2, 3, ... n)
bool db::rearrangeEmployeeIDs() {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(dbPath.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE EMPLOYEE_IDS failed (open) db=" + dbPath);
        return false;
    }

    // Start a transaction for data consistency
    char* err = nullptr;
    if (sqlite3_exec(dbPtr, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE EMPLOYEE_IDS failed (begin)");
        return false;
    }

    // Create a temporary table with sequential IDs
    const char* createTemp =
        "CREATE TEMPORARY TABLE TEMP_EMPLOYEES AS "
        "SELECT ROW_NUMBER() OVER (ORDER BY EMPLOYEE_ID) AS NEW_ID, "
        "NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE "
        "FROM EMPLOYEES;";

    if (sqlite3_exec(dbPtr, createTemp, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE EMPLOYEE_IDS failed (temp)");
        return false;
    }

    // Delete all rows from the original table
    if (sqlite3_exec(dbPtr, "DELETE FROM EMPLOYEES;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE EMPLOYEE_IDS failed (clear)");
        return false;
    }

    // Copy data back with new sequential IDs
    const char* copyBack =
        "INSERT INTO EMPLOYEES (EMPLOYEE_ID, NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE) "
        "SELECT NEW_ID, NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE FROM TEMP_EMPLOYEES;";

    if (sqlite3_exec(dbPtr, copyBack, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE EMPLOYEE_IDS failed (copy)");
        return false;
    }

    // Commit the transaction
    if (sqlite3_exec(dbPtr, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE EMPLOYEE_IDS failed (commit)");
        return false;
    }

    sqlite3_close(dbPtr);
    system::logMessage(system::messageClassification::INFO, "DB REARRANGE EMPLOYEE_IDS ok");
    return true;
}

bool db::checkProjectChanges() {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(dbPath.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return false;
    }

    // Query to check if there are gaps in project IDs (PRJ-xxxxx format)
    sqlite3_stmt* stmt = nullptr;
    bool hasGaps = false;

    if (const auto sql = "SELECT PROJECT_ID FROM PROJECT_LIST ORDER BY PROJECT_ID;"; sqlite3_prepare_v2(dbPtr, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        int expectedId = 1;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* projectIdText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (projectIdText) {
                // Extract numeric part from PRJ-xxxxx
                std::string idStr(projectIdText);
                if (idStr.length() >= 4 && idStr.substr(0, 4) == "PRJ-") {
                    try {
                        int currentId = std::stoi(idStr.substr(4));
                        if (currentId != expectedId) {
                            hasGaps = true;
                            break;
                        }
                    } catch (...) {
                        // Invalid format, consider it a gap
                        hasGaps = true;
                        break;
                    }
                }
            }
            expectedId++;
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(dbPtr);
    return hasGaps;
}

bool db::rearrangeProjectIDs() {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(dbPath.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE PROJECT_IDS failed (open) db=" + dbPath);
        return false;
    }

    // Start a transaction for data consistency
    char* err = nullptr;
    if (sqlite3_exec(dbPtr, "BEGIN TRANSACTION;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE PROJECT_IDS failed (begin)");
        return false;
    }

    // Create a temporary table with sequential IDs in PRJ-xxxxx format
    // Using PRINTF to format as PRJ-00001, PRJ-00002, etc.
    const char* createTemp =
        "CREATE TEMPORARY TABLE TEMP_PROJECTS AS "
        "SELECT PRINTF('PRJ-%05d', ROW_NUMBER() OVER (ORDER BY PROJECT_ID)) AS NEW_ID, "
        "PROJECT_NAME, STATUS, START_DATE, NOTE "
        "FROM PROJECT_LIST;";

    if (sqlite3_exec(dbPtr, createTemp, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE PROJECT_IDS failed (temp)");
        return false;
    }

    // Delete all rows from the original table
    if (sqlite3_exec(dbPtr, "DELETE FROM PROJECT_LIST;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE PROJECT_IDS failed (clear)");
        return false;
    }

    // Copy data back with new sequential IDs
    const char* copyBack =
        "INSERT INTO PROJECT_LIST (PROJECT_ID, PROJECT_NAME, STATUS, START_DATE, NOTE) "
        "SELECT NEW_ID, PROJECT_NAME, STATUS, START_DATE, NOTE FROM TEMP_PROJECTS;";

    if (sqlite3_exec(dbPtr, copyBack, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE PROJECT_IDS failed (copy)");
        return false;
    }

    // Commit the transaction
    if (sqlite3_exec(dbPtr, "COMMIT;", nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_exec(dbPtr, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB REARRANGE PROJECT_IDS failed (commit)");
        return false;
    }

    sqlite3_close(dbPtr);
    system::logMessage(system::messageClassification::INFO, "DB REARRANGE PROJECT_IDS ok");
    return true;
}

// Example usage (as a code comment):
// std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
// // Fetch NAME for first employee: row=1, col=2 (1-based indices)
// std::string name = db::fetchCell(dbPath, 1, 2);
// if (name.empty()) {
//     // empty string means either the cell is NULL/empty OR an error occured (e.g., row/col out-of-range)
//     // To disambiguate, you might check the ID column:
//     // std::string id = db::fetchCell(dbPath, 1, 1);
//     // if (id.empty()) { /* row doesn't exist or error */ } else { /* name is empty */ }
// } else {
//     // use 'name'
// }

// Fetch a single cell value by 1-based row and column indices
std::string db::fetchCell(const std::string& p_dbName, const size_t p_row, const size_t p_col) {
    if (p_row == 0 || p_col == 0) return "";

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        return "";
    }

    // Normalize to filename only for checking
    std::string filename = p_dbName;
    if (const size_t pos = filename.find_last_of("/\\"); pos != std::string::npos) {
        filename = filename.substr(pos + 1);
    }

    std::string sql = "SELECT * FROM ";
    if (p_dbName == appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll) {
        sql += "EMPLOYEES";
    } else if (p_dbName == appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject) {
        sql += "PROJECT_LIST";
    } else if (filename.find("PRJ-") != std::string::npos && filename.find(".db") != std::string::npos) {
        // Material database (expense/PRJ-xxxxx.db)
        sql += "MATERIALS";
    } else if (
        // SIMPLIFIED: Attendance database - has hyphen, not PRJ, has .db
        filename.find("-") != std::string::npos &&
        filename.find(".db") != std::string::npos &&
        filename.find("PRJ") == std::string::npos) {
        // Attendance database
        sql += "WEEKLY_ATTENDANCE";
    } else {
        sqlite3_close(dbPtr);
        return "";
    }

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(dbPtr, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(dbPtr);
        return "";
    }

    if (p_col - 1 >= static_cast<size_t>(sqlite3_column_count(stmt))) {
        sqlite3_finalize(stmt);
        sqlite3_close(dbPtr);
        return "";
    }

    std::string result;
    for (size_t row = 0; sqlite3_step(stmt) == SQLITE_ROW && ++row <= p_row;) {
        if (row == p_row) {
            const auto cell = reinterpret_cast<const char*>(sqlite3_column_text(stmt, static_cast<int>(p_col - 1)));
            result = cell ? cell : "";
            break;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(dbPtr);
    return result;
}

// Ensure WEEKLY_ATTENDANCE table exists (composite PK) regardless of db name
bool db::ensureWeeklyAttendanceTable(const std::string& p_dbName) {
    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB ENSURE WEEKLY_ATTENDANCE failed (open) db=" + p_dbName);
        return false;
    }

    const char* createSql =
        "CREATE TABLE IF NOT EXISTS WEEKLY_ATTENDANCE ("
        "EMPLOYEE_ID TEXT NOT NULL,"
        "WEEK_START TEXT NOT NULL,"
        "SUN REAL NOT NULL,"
        "MON REAL NOT NULL,"
        "TUE REAL NOT NULL,"
        "WED REAL NOT NULL,"
        "THU REAL NOT NULL,"
        "FRI REAL NOT NULL,"
        "SAT REAL NOT NULL,"
        "PRIMARY KEY (EMPLOYEE_ID, WEEK_START)"
        ");";

    char* err = nullptr;
    const int rc = sqlite3_exec(dbPtr, createSql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    if (rc == SQLITE_OK) {
        system::logMessage(system::messageClassification::INFO, "DB ENSURE WEEKLY_ATTENDANCE ok db=" + p_dbName);
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB ENSURE WEEKLY_ATTENDANCE failed db=" + p_dbName);
    return false;
}

// Insert or upsert a weekly attendance row. valuesCsv should be:
//  '\''EMP-00001'\'', '\''2025-01-05'\'', 8, 8, 8, 8, 8, 0, 0
bool db::insertWeeklyAttendance(const std::string& p_dbName, const std::string& valuesCsv) {
    // Ensure table exists
    if (!ensureWeeklyAttendanceTable(p_dbName)) return false;

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB ATTENDANCE INSERT failed (open) db=" + p_dbName);
        return false;
    }

    const std::string sql =
        "INSERT INTO WEEKLY_ATTENDANCE (EMPLOYEE_ID, WEEK_START, SUN, MON, TUE, WED, THU, FRI, SAT) VALUES (" + valuesCsv + ") "
        "ON CONFLICT(EMPLOYEE_ID, WEEK_START) DO UPDATE SET "
        "SUN=excluded.SUN, MON=excluded.MON, TUE=excluded.TUE, WED=excluded.WED, THU=excluded.THU, FRI=excluded.FRI, SAT=excluded.SAT;";

    char* err = nullptr;
    const int rc = sqlite3_exec(dbPtr, sql.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    if (rc == SQLITE_OK) {
        system::logMessage(system::messageClassification::INFO, "DB ATTENDANCE INSERT ok db=" + p_dbName);
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB ATTENDANCE INSERT failed db=" + p_dbName);
    return false;
}

bool db::updateWeeklyAttendanceRow(const std::string& p_dbName, const std::string& employeeId, const std::string& weekStartIso, const std::string& setClause) {
    // Ensure table exists
    if (!ensureWeeklyAttendanceTable(p_dbName)) return false;

    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB ATTENDANCE UPDATE failed (open) db=" + p_dbName);
        return false;
    }
    const std::string sql = "UPDATE WEEKLY_ATTENDANCE SET " + setClause +
                            " WHERE EMPLOYEE_ID = '" + employeeId + "' AND WEEK_START='" + weekStartIso + "';";
    char* err = nullptr;
    const int rc = sqlite3_exec(dbPtr, sql.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    if (rc == SQLITE_OK) {
        system::logMessage(system::messageClassification::INFO, "DB ATTENDANCE UPDATE ok db=" + p_dbName + " key=provided");
        return true;
    }
    system::logMessage(system::messageClassification::ERROR, "DB ATTENDANCE UPDATE failed db=" + p_dbName);
    return false;
}

bool db::deleteWeeklyAttendanceRow(const std::string& p_dbName, const std::string& employeeId, const std::string& weekStartIso) {
    if (!ensureWeeklyAttendanceTable(p_dbName)) return false;
    sqlite3* dbPtr = nullptr;
    if (sqlite3_open(p_dbName.c_str(), &dbPtr) != SQLITE_OK) {
        if (dbPtr) sqlite3_close(dbPtr);
        system::logMessage(system::messageClassification::ERROR, "DB ATTENDANCE DELETE failed (open) db=" + p_dbName);
        return false;
    }
    const std::string sql = "DELETE FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID='" + employeeId + "' AND WEEK_START='" + weekStartIso + "';";
    char* err = nullptr;
    const int rc = sqlite3_exec(dbPtr, sql.c_str(), nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(dbPtr);
    if (rc == SQLITE_OK) {
        system::logMessage(system::messageClassification::INFO, "DB ATTENDANCE DELETE ok db=" + p_dbName + " key=provided");
        return true;
    }
    system::logMessage(system::messageClassification::WARNING, "DB ATTENDANCE DELETE no-op or failed db=" + p_dbName);
    return false;
}
