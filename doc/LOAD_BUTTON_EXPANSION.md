# Load Button Feature Expansion

## Overview
Extended the "Load" button functionality from Employee Management to all other management sections (Weekly Attendance and Materials) for consistent user experience across the application.

**Date:** December 18, 2024

## Features Added

### 1. Load Weekly Attendance Button
**Location:** Monitor UI > Weekly Attendance Management

**Functionality:**
- Fetches attendance records from the `WEEKLY_ATTENDANCE` table in the payroll database
- Requires both Employee ID and Week selection
- Populates all daily hours fields (Sunday through Saturday)
- Uses direct SQLite3 queries for data retrieval

**User Workflow:**
1. Enter Employee ID (e.g., "EMP-00001" or just "1")
2. Select Week from dropdown
3. Click "Load Weekly Attendance"
4. All daily hours fields populate automatically
5. Edit as needed and click "Update Weekly Attendance"

**Technical Details:**
```cpp
// Query used
const char* sql = "SELECT SUN, MON, TUE, WED, THU, FRI, SAT 
                  FROM WEEKLY_ATTENDANCE 
                  WHERE EMPLOYEE_ID = ? AND WEEK_START = ? 
                  LIMIT 1;";
```

**Benefits:**
- ✅ Quick review of existing attendance records
- ✅ Easy correction of attendance data
- ✅ No need to remember previous entries
- ✅ Consistent with employee management workflow

---

### 2. Load Material Button
**Location:** Monitor UI > Materials Management

**Functionality:**
- Retrieves material information from project expense databases
- Requires both Project ID and Material ID
- Populates Material Name, Quantity, and Unit Price
- Uses `db::fetchCell()` helper function

**User Workflow:**
1. Enter Project ID (e.g., "PRJ-00001" or just "1")
2. Enter Material ID
3. Click "Load Material"
4. Material Name, Quantity, and Unit Price fields populate automatically
5. Edit as needed and click "Update Material"

**Technical Details:**
```cpp
// Database path
const std::string expenseDbPath = 
    appConfig::g_dataDirectory + 
    appConfig::g_projectDirectory +
    appConfig::g_projectExpenseDirectory + 
    validatedProjectID + ".db";

// Columns fetched
// Column 1: MATERIAL_ID
// Column 2: MATERIAL_NAME
// Column 3: QUANTITY
// Column 4: UNIT_PRICE
```

**Benefits:**
- ✅ Fast retrieval of material details
- ✅ Accurate updates without retyping
- ✅ Prevents errors from manual entry
- ✅ Matches project management pattern

---

## Implementation Details

### Code Changes

#### 1. `src/ui/ui.cpp`
- **Added:** `#include "../../dependencies/sqlite/sqlite3.h"` for database operations
- **Added:** Load Weekly Attendance button in Attendance Management section
- **Added:** Load Material button in Materials Management section
- **Modified:** Button layouts to accommodate new Load buttons

#### 2. Button Layout Consistency
All management sections now follow the same 4-button pattern:

```
┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│ Add New         │ Update          │ Delete          │ Load            │
└─────────────────┴─────────────────┴─────────────────┴─────────────────┘
```

**Button Specifications:**
- Width: 350.0f pixels (Attendance & Materials) / 260.0f (Employee & Project)
- Height: 40.0f pixels
- Spacing: `ImGui::SameLine()` for horizontal layout

### Database Operations

#### Weekly Attendance
**Query Type:** Direct SQLite3 prepared statement
**Parameters:**
- Employee ID (integer)
- Week Start Date (text, ISO format YYYY-MM-DD)

**Fields Loaded:**
1. Sunday Hours (REAL)
2. Monday Hours (REAL)
3. Tuesday Hours (REAL)
4. Wednesday Hours (REAL)
5. Thursday Hours (REAL)
6. Friday Hours (REAL)
7. Saturday Hours (REAL)

#### Materials
**Query Type:** Row-by-row scan using `db::fetchCell()`
**Parameters:**
- Project ID (PRJ-XXXXX format)
- Material ID (any string)

**Fields Loaded:**
1. Material Name (TEXT)
2. Quantity (REAL)
3. Unit Price (REAL)

### Error Handling

#### Weekly Attendance
- ✅ Validates Employee ID format
- ✅ Validates Week selection
- ✅ Checks if database exists and is accessible
- ✅ Provides clear error messages if record not found
- ✅ Properly closes database connections

#### Materials
- ✅ Validates Project ID format (PRJ-XXXXX)
- ✅ Validates Material ID is not empty
- ✅ Checks if project expense database exists
- ✅ Scans up to 1000 rows for material
- ✅ Reports if material not found in project

### User Feedback

All Load buttons provide system log messages:

**Success Messages:**
- `"Attendance: Weekly attendance loaded successfully for EMP-00001"`
- `"DB: Material data loaded successfully for MAT-001"`

**Warning Messages:**
- `"Attendance: Employee ID and Week are required to load attendance data."`
- `"Attendance: No attendance record found for Employee EMP-00001 for week 2024-01-07"`
- `"DB: Material MAT-001 not found in project PRJ-00001"`

**Error Messages:**
- `"Attendance: Database query failed"`
- `"Attendance: Failed to open database"`
- `"DB: Project expense database not found for PRJ-00001"`

---

## Comparison Table

| Management Section | Add Button | Update Button | Delete Button | Load Button |
|-------------------|------------|---------------|---------------|-------------|
| **Employee**      | ✅         | ✅            | ✅            | ✅ (existing) |
| **Weekly Attendance** | ✅     | ✅            | ✅            | ✅ **NEW** |
| **Project**       | ✅         | ✅            | ✅            | ✅ (existing) |
| **Materials**     | ✅         | ✅            | ✅            | ✅ **NEW** |

---

## Benefits

### For Users
1. **Consistency** - Same workflow across all management sections
2. **Efficiency** - No need to manually retype existing data
3. **Accuracy** - Loading from database prevents typos
4. **Convenience** - Quick data review and editing

### For Developers
1. **Code Reuse** - Similar patterns across all Load buttons
2. **Maintainability** - Consistent error handling
3. **Extensibility** - Easy to add more Load buttons if needed

---

## Testing

### Test Cases

#### Weekly Attendance Load
1. ✅ Load existing attendance record - Success
2. ✅ Load with invalid Employee ID - Error message shown
3. ✅ Load with no week selected - Error message shown
4. ✅ Load non-existent record - Warning message shown
5. ✅ Load with database unavailable - Error message shown

#### Material Load
1. ✅ Load existing material - Success
2. ✅ Load with invalid Project ID - Error message shown
3. ✅ Load with empty Material ID - Error message shown
4. ✅ Load from non-existent project - Warning message shown
5. ✅ Load non-existent material - Warning message shown

---

## Future Enhancements

Potential improvements for future versions:

1. **Auto-Load on Select** - Load data automatically when ID is entered
2. **Quick Load Dropdown** - Select from recent/common items
3. **Batch Load** - Load multiple records at once
4. **Load History** - Track recently loaded items
5. **Smart Suggestions** - Auto-complete based on database contents

---

## Related Features

- [Load Employee Data Feature](LOAD_EMPLOYEE_DATA_FEATURE.md) - Original Load button implementation
- [Input Validation System](INPUT_VALIDATION_SYSTEM.md) - Validation used by Load buttons
- [Database Module](modules/handler-db.md) - Database operations

---

## Files Modified

1. **`src/ui/ui.cpp`**
   - Added `sqlite3.h` include
   - Implemented Load Weekly Attendance button (lines ~1389-1451)
   - Implemented Load Material button (lines ~2030-2090)

2. **`doc/LOAD_BUTTON_EXPANSION.md`** (NEW)
   - This documentation file

3. **`CHANGELOG.md`**
   - Added entry for Load Button Feature Expansion

---

## Database Schema Reference

### WEEKLY_ATTENDANCE Table
```sql
CREATE TABLE IF NOT EXISTS WEEKLY_ATTENDANCE (
    EMPLOYEE_ID INTEGER NOT NULL,
    WEEK_START TEXT NOT NULL,
    SUN REAL NOT NULL,
    MON REAL NOT NULL,
    TUE REAL NOT NULL,
    WED REAL NOT NULL,
    THU REAL NOT NULL,
    FRI REAL NOT NULL,
    SAT REAL NOT NULL,
    PRIMARY KEY (EMPLOYEE_ID, WEEK_START)
);
```

### MATERIALS Table
```sql
CREATE TABLE IF NOT EXISTS MATERIALS (
    MATERIAL_ID TEXT PRIMARY KEY,
    MATERIAL_NAME TEXT NOT NULL,
    QUANTITY REAL NOT NULL,
    UNIT_PRICE REAL NOT NULL
);
```

---

## Summary

The Load button feature has been successfully expanded to all management sections, providing users with a consistent and efficient workflow for viewing and editing existing data. This enhancement reduces manual data entry, minimizes errors, and improves overall user experience across the application.

**Total Lines Added:** ~120 lines
**Sections Enhanced:** 2 (Weekly Attendance, Materials)
**User Benefits:** Faster editing, fewer errors, consistent interface

