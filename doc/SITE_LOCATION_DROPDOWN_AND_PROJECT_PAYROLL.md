# Site Location Dropdown and Project Payroll Integration

## Overview
This document describes the implementation of a dynamic site location dropdown in Employee Management and automatic project payroll expense tracking.

## Changes Made

### 1. Database Schema Updates (`src/handler/db.cpp`)
- Added `PAYROLL_EXPENSES` table to project expense databases (PRJ-xxxxx.db)
- Table structure:
  ```sql
  CREATE TABLE IF NOT EXISTS PAYROLL_EXPENSES (
      EMPLOYEE_ID INTEGER PRIMARY KEY,
      EMPLOYEE_NAME TEXT NOT NULL,
      POSITION TEXT NOT NULL,
      HOURLY_RATE REAL NOT NULL,
      TOTAL_HOURS REAL NOT NULL DEFAULT 0,
      TOTAL_COST REAL NOT NULL DEFAULT 0
  );
  ```

### 2. New Database Functions (`src/handler/db.h` and `src/handler/db.cpp`)
Added three new functions for managing project payroll expenses:
- `db::insertPayrollExpense()` - Insert/upsert employee into project payroll
- `db::updatePayrollExpense()` - Update employee hours and costs in project
- `db::deletePayrollExpense()` - Remove employee from project payroll

### 3. Monitor Module Enhancements (`src/core/monitor.h` and `src/core/monitor.cpp`)

#### New Functions:
- `monitor::listProjectIDs()` - Fetch all project IDs for dropdown
- `monitor::addEmployeeToProject()` - Add employee to project payroll expenses
- `monitor::removeEmployeeFromProject()` - Remove employee from project payroll
- `monitor::updateEmployeeProjectHours()` - Update employee hours/costs in project

#### Enhanced Functions:
- `monitor::addEmployee()` - Now checks if site location is a project and adds employee to project payroll
- `monitor::updateEmployee()` - Detects site location changes and manages project payroll:
  - Removes from old project if previous location was a project
  - Adds to new project if new location is a project
- `monitor::deleteEmployee()` - Removes employee from project payroll if assigned to a project

### 4. UI Updates (`src/ui/ui.cpp`)
Replaced text input for Site Location with a dropdown containing:
- "Main Office" (static option)
- "Warehouse" (static option)
- All project IDs from database (dynamically loaded)

Features:
- Auto-refreshes project list every 60 frames to stay current
- Maintains selection state across renders
- Updates employee location when dropdown selection changes

## Usage

### For Users

1. **Adding/Editing Employees:**
   - Select site location from dropdown instead of typing
   - If you select a project (e.g., "PRJ-00001"), the employee is automatically added to that project's payroll expenses
   - Changing an employee's location between projects automatically moves their payroll expense

2. **Project Payroll Tracking:**
   - Each project's expense database (expense/PRJ-xxxxx.db) now tracks employee payroll
   - PAYROLL_EXPENSES table stores: employee ID, name, position, hourly rate, total hours, and total cost
   - This allows project-specific labor cost tracking

### For Developers

**To update employee hours in a project:**
```cpp
monitor::updateEmployeeProjectHours("PRJ-00001", "1", 40.0, 2000.0);
```

**To manually add an employee to a project:**
```cpp
monitor::addEmployeeToProject("PRJ-00001", "1", "John Doe", "Engineer", "50.0");
```

**To remove an employee from a project:**
```cpp
monitor::removeEmployeeFromProject("PRJ-00001", "1");
```

## Benefits

1. **Improved UX:** Dropdown prevents typos and ensures consistency
2. **Dynamic:** Automatically includes all projects from database
3. **Automated Tracking:** Employee payroll automatically added to project expenses
4. **Data Integrity:** Site location changes properly update project assignments
5. **Scalable:** Works with any number of projects

## Future Enhancements

Potential improvements for future iterations:
- Weekly automatic calculation of employee hours from attendance data
- Project payroll cost summaries in project reports
- Payroll expense visualization in project management UI
- Historical tracking of employee project assignments
- Multi-project assignment support (if needed)

## Technical Notes

- Project detection: Any site location starting with "PRJ-" is treated as a project
- Database operations use INSERT ... ON CONFLICT for upsert behavior
- Payroll expense tables are automatically created when project databases are initialized
- The dropdown refreshes every 60 frames to balance responsiveness and performance

