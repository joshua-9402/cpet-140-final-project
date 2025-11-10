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
 * - !!!!!!!!!! THIS NEEDS A REWORK ASAP !!!!!!!!!!
 */


#include "db.h"
#include <sqlite3.h>


/*
 * This method will launch SQLite
 */


bool launchSQLITE() {
 return true;
}


bool createDatabase() {
 return true;
}
