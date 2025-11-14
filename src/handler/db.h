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

// File operations
bool createFileText(const std::string& p_filename);
std::string readFileText(const std::string& p_filename, int p_lineFileText);
bool appendFileText(const std::string& p_filename, const std::string& p_newText, bool addNewline = true);

// Database operations
bool createDatabase(const std::string& p_dbName);
bool openDatabase(const std::string& p_dbName);
bool closeDatabase(const std::string& p_dbName);
bool appendDatabase(const std::string& p_dbName, std::string& p_data);

#endif // DB_H

