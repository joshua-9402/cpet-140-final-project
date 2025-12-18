# Load Employee Data Feature

## Overview
Added a "Load Employee" button in the Employee Management section that fetches employee information from the database and automatically populates all input fields.

## Location
**UI Section:** Monitor > Employee Management  
**File:** `src/ui/ui.cpp`

## Feature Description

### Button Placement
The "Load Employee" button is located:
- On the same row as the other action buttons (Add New Employee, Update Employee, Delete Employee)
- Fourth button from the left in the action button row
- All buttons are uniformly sized (260px width, 40px height)

### Button Layout
```
┌─────────────────┬─────────────────┬─────────────────┬─────────────────┐
│ Add New Employee│ Update Employee │ Delete Employee │ Load Employee   │
└─────────────────┴─────────────────┴─────────────────┴─────────────────┘
```

### Functionality

1. **Input Validation**
   - Validates the Employee ID field before attempting to load data
   - Shows warning if Employee ID is empty or invalid

2. **Database Lookup**
   - Searches the payroll database for the specified Employee ID
   - Scans up to 1000 employee records for a match

3. **Field Population**
   - When employee is found, automatically fills in:
     - Name
     - Position
     - Site Location (including updating the dropdown selection)
     - Hourly Rate
     - Hours Worked
     - Advance
   - Logs success message with employee ID and name

4. **Error Handling**
   - Shows warning message if employee ID is not found
   - Maintains existing field values if lookup fails

### Usage Workflow

**Quick Edit Workflow:**
1. Enter the Employee ID in the "Employee ID" field
2. Click "Load Employee"
3. All fields are populated with current database values
4. Modify any fields as needed
5. Click "Update Employee" to save changes

**Quick View Workflow:**
1. Enter an Employee ID
2. Click "Load Employee"
3. View all employee details in the input fields

## Technical Implementation

### Database Access
```cpp
const std::string dbPath = appConfig::g_dataDirectory + 
                          appConfig::g_payrollDirectory + 
                          appConfig::g_dbNamePayroll;
```

### Data Retrieval
Uses `db::fetchCell()` to retrieve each field:
- Column 1: Employee ID
- Column 2: Name
- Column 3: Position
- Column 4: Site Location
- Column 5: Salary (Hourly Rate)
- Column 6: Hours Worked
- Column 7: Advance

### Site Location Dropdown Sync
The button intelligently updates the site location dropdown to match the loaded employee's location:
```cpp
for (size_t i = 0; i < siteLocationOptions.size(); ++i) {
    if (siteLocationOptions[i] == empLocation) {
        selectedLocationIndex = static_cast<int>(i);
        break;
    }
}
```

## Benefits

1. **Faster Updates**: No need to manually retype all employee data when updating a single field
2. **Reduced Errors**: Prevents typos when editing existing employee records
3. **Quick Reference**: Easy way to view employee details without scrolling through the database viewer
4. **Improved UX**: Streamlines the employee update workflow

## User Messages

### Success Message
```
DB: Loaded employee data for ID [ID] ([Employee Name])
```

### Warning Messages
```
DB: Valid Employee ID is required to load employee data.
DB: Employee ID [ID] not found in database.
```

## Example Usage Scenarios

### Scenario 1: Update Employee's Site Location
1. Enter Employee ID: `5`
2. Click "Load Employee"
3. Change Site Location dropdown from "Main Office" to "PRJ-00001"
4. Click "Update Employee"
5. Employee is automatically added to project PRJ-00001 payroll expenses

### Scenario 2: Adjust Employee's Hourly Rate
1. Enter Employee ID: `12`
2. Click "Load Employee"
3. Change Hourly Rate from `50.00` to `55.00`
4. Click "Update Employee"
5. Rate is updated in database

### Scenario 3: Check Employee Details
1. Enter Employee ID: `8`
2. Click "Load Employee"
3. Review all employee information without making changes

## Integration with Existing Features

- **Works seamlessly with Site Location Dropdown**: Loaded location properly selects the correct dropdown option
- **Compatible with Input Validation**: All loaded values are subject to validation when updating
- **Integrates with Project Payroll**: Changing site location to/from a project still triggers automatic payroll expense management

## Future Enhancements

Potential improvements:
- Add "Clear Fields" button to reset all input fields
- Add keyboard shortcut (e.g., Ctrl+L) to load employee data
- Add autocomplete dropdown for Employee ID field
- Add "Load Next/Previous Employee" buttons for quick browsing

