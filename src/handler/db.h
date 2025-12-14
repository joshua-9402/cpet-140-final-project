/*
 * CpET 140 Final Project — Database module header
 *
 * Contributors: Joshua Literal
 *
 * Purpose
 * - Declares persistence functions for database operations
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
        static bool openDatabase(const std::string& p_dbName);
        static bool closeDatabase();
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
};
#endif // DB_H
