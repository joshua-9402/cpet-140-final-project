#ifndef CPET_140_FINAL_PROJECT_DB_H
#define CPET_140_FINAL_PROJECT_DB_H

/**
 * SQLite utility helpers.
 * - Availability check (no side effects)
 * - Lightweight database file creation (no schema creation)
 */

#include <string>

class db {
public:
    static bool createFileText(const std::string& p_filename);
    static std::string readFileText(const std::string& p_filename, const int p_lineFileText);


    static bool createDatabase(const std::string& p_dbName);
    static bool openDatabase(const std::string& p_dbName);
    static bool closeDatabase(const std::string& p_dbName);
};


#endif //CPET_140_FINAL_PROJECT_DB_H