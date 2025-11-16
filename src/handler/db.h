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

class db {
    // Database-related declarations can go here if needed
    public:
        // Database operations
        static bool createDatabase(const std::string& p_dbName);
        static bool searchDatabase(const std::string& p_dbName);
        static bool openDatabase(const std::string& p_dbName);
        static bool closeDatabase(const std::string& p_dbName);
        static bool appendToDatabase(const std::string& p_dbName, std::string& p_data);
};
#endif // DB_H
