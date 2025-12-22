/*
 * CpET 140 Final Project — Database module header
 * StructuraCost - Handler - Database module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Provides persistence adapters for payroll and project management
 * - Handle database creation, operations, and data retrieval
 *
 * Boundaries
 * - No rendering and no direct UI calls
 * - Storage details are hidden behind db.h interface
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if used from workers
 * - Prefer explicit status results and avoid partial writes on failure
 * - Supports SQLite database operations with encryption
 */

#ifndef DB_H
#define DB_H

#include <string>
#include <vector>

class db {
    // Database-related declarations can go here if needed
    public:
        // Database operations
        static bool createDatabase(const std::string& p_dbName);
        static bool appendDatabase(const std::string& p_dbName, const std::string& p_data);
        static bool updateDatabase(const std::string& p_dbName, const std::string& p_id, const std::string& p_data);
        static bool isSQLiteAvailable();
        static bool deleteRow(const std::string &p_dbName, const std::string &p_TableRow);
        static bool checkEmployeeChanges();
        static bool rearrangeEmployeeIDs();
        static bool checkProjectChanges();
        static bool rearrangeProjectIDs();

        // Fetch a single cell value. p_row and p_col are 1-based indices.
        // Returns the cell value as a string, or empty string on error.
        static std::string fetchCell(const std::string& p_dbName, size_t p_row, size_t p_col);

        // Weekly attendance explicit operations (works for any DB path, including base_payroll.db)
        static bool ensureWeeklyAttendanceTable(const std::string& p_dbName);
        static bool insertWeeklyAttendance(const std::string& p_dbName, const std::string& valuesCsv);
        static bool updateWeeklyAttendanceRow(const std::string& p_dbName, const std::string& employeeId, const std::string& weekStartIso, const std::string& setClause);
        static bool deleteWeeklyAttendanceRow(const std::string& p_dbName, const std::string& employeeId, const std::string& weekStartIso);

        // Project payroll expenses operations
        static bool insertPayrollExpense(const std::string& p_dbName, const std::string& valuesCsv);
        static bool updatePayrollExpense(const std::string& p_dbName, const std::string& employeeId, const std::string& setClause);
        static bool deletePayrollExpense(const std::string& p_dbName, const std::string& employeeId);
};
#endif // DB_H
