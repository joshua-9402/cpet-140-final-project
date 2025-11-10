#ifndef CPET_140_FINAL_PROJECT_DB_H
#define CPET_140_FINAL_PROJECT_DB_H

/**
 * SQLite utility helpers.
 * - Availability check (no side effects)
 * - Lightweight database file creation (no schema creation)
 */

class db {
public:
    static bool launchSQLITE();
    static bool createDatabase();
};


#endif //CPET_140_FINAL_PROJECT_DB_H