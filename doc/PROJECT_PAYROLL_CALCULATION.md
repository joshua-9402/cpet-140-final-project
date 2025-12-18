# Automatic Project Payroll Cost Calculation

## Overview
The system now automatically calculates project labor costs by summing weekly attendance hours for all employees assigned to each project and computing the total cost based on their hourly rates.

**Date:** December 18, 2024

## How It Works

### Data Flow
1. **Employee Assignment**: Employee's site location is set to a project (e.g., "PRJ-00001")
2. **Attendance Tracking**: Weekly hours are recorded in WEEKLY_ATTENDANCE table
3. **Payroll Calculation**: System sums all attendance hours and calculates cost
4. **Project Expenses**: Results are stored in project's PAYROLL_EXPENSES table

### Calculation Formula
```
Total Hours = SUM(All weekly attendance hours for employee)
Total Cost = Total Hours × Hourly Rate
```

## User Interface

### Calculate Button
**Location:** Monitor UI > Top of page  
**Button:** "Calculate All Project Payroll Costs"  
**Helper Text:** "Updates total hours & costs for all employees in projects from weekly attendance"

### What Happens When Clicked
1. System scans all employees in the payroll database
2. For each employee assigned to a project (site location = "PRJ-XXXXX"):
   - Queries all weekly attendance records
   - Sums total hours: SUN + MON + TUE + WED + THU + FRI + SAT (across all weeks)
   - Multiplies by employee's hourly rate
   - Updates project's PAYROLL_EXPENSES table
3. Logs results for each employee updated
4. Shows summary message

## Technical Implementation

### New Functions

#### `monitor::calculateProjectPayrollCosts(projectId)`
Calculates payroll costs for a single project.

**Parameters:**
- `projectId` - Project ID (e.g., "PRJ-00001")

**Process:**
1. Opens project expense database
2. Finds all employees with site location = projectId
3. For each employee:
   - Queries: `SELECT SUM(SUN+MON+TUE+WED+THU+FRI+SAT) FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID = ?`
   - Calculates: `totalCost = totalHours × hourlyRate`
   - Updates: `PAYROLL_EXPENSES` table with new totals

**Returns:** `true` if successful, `false` otherwise

#### `monitor::calculateAllProjectPayrollCosts()`
Calculates payroll costs for all projects.

**Process:**
1. Gets list of all project IDs using `listProjectIDs()`
2. Calls `calculateProjectPayrollCosts()` for each project
3. Counts successful updates
4. Logs summary

**Returns:** `true` if any projects were updated, `false` otherwise

### Database Schema

#### PAYROLL_EXPENSES Table
```sql
CREATE TABLE IF NOT EXISTS PAYROLL_EXPENSES (
    EMPLOYEE_ID INTEGER PRIMARY KEY,
    EMPLOYEE_NAME TEXT NOT NULL,
    POSITION TEXT NOT NULL,
    HOURLY_RATE REAL NOT NULL,
    TOTAL_HOURS REAL NOT NULL DEFAULT 0,    -- Updated by calculation
    TOTAL_COST REAL NOT NULL DEFAULT 0      -- Updated by calculation
);
```

### SQL Query Used
```sql
SELECT SUM(SUN + MON + TUE + WED + THU + FRI + SAT) 
FROM WEEKLY_ATTENDANCE 
WHERE EMPLOYEE_ID = ?;
```

## Usage Examples

### Example 1: Manual Calculation
**Scenario:** Added new attendance data, need to update project costs

**Steps:**
1. Navigate to Monitor UI
2. Click "Calculate All Project Payroll Costs"
3. Check logs for results

**Result:**
```
Updated payroll for Employee 5 in project PRJ-00001: 160.5 hours, ₱8025.00
Updated payroll for Employee 8 in project PRJ-00001: 120.0 hours, ₱6000.00
Updated payroll for Employee 12 in project PRJ-00002: 80.0 hours, ₱4800.00
Updated payroll costs for 2 project(s)
```

### Example 2: After Adding Weekly Attendance
**Workflow:**
1. Add/update weekly attendance for employees
2. Click "Calculate All Project Payroll Costs"
3. Project expenses are automatically updated
4. Can now print project reports with accurate labor costs

### Example 3: Verification
**Check if costs are calculated:**
1. Click "Calculate All Project Payroll Costs"
2. Navigate to Project Management
3. View project expense database
4. PAYROLL_EXPENSES table shows updated TOTAL_HOURS and TOTAL_COST

## Calculation Details

### Multi-Week Example
**Employee:** John Doe (ID: 5)  
**Hourly Rate:** ₱50.00  
**Site Location:** PRJ-00001

**Weekly Attendance:**
| Week Starting | Sun | Mon | Tue | Wed | Thu | Fri | Sat | Total |
|--------------|-----|-----|-----|-----|-----|-----|-----|-------|
| 2024-12-01   | 0   | 8   | 8   | 8   | 8   | 8   | 0   | 40    |
| 2024-12-08   | 0   | 8   | 8   | 8   | 8   | 8   | 0   | 40    |
| 2024-12-15   | 0   | 8   | 8   | 8.5 | 8   | 8   | 0   | 40.5  |

**Calculation:**
```
Total Hours = 40 + 40 + 40.5 = 120.5 hours
Total Cost = 120.5 × ₱50.00 = ₱6,025.00
```

**Database Update:**
```sql
UPDATE PAYROLL_EXPENSES 
SET TOTAL_HOURS = 120.5, TOTAL_COST = 6025.00 
WHERE EMPLOYEE_ID = 5;
```

## Integration Points

### Works With
- ✅ **Employee Management**: Site location assignment triggers payroll entry
- ✅ **Weekly Attendance**: All attendance data is included in calculation
- ✅ **Project Management**: Load Project button works with expense data
- ✅ **Print Module**: Project reports can show payroll expenses

### Automatic Updates Needed After
- Adding weekly attendance data
- Changing employee hourly rates
- Moving employees between projects
- Updating past attendance records

## Benefits

1. **Accurate Labor Costing**: Real labor costs based on actual hours worked
2. **Time Savings**: No manual calculation needed
3. **Multi-Week Support**: Automatically sums across all recorded weeks
4. **Real-Time Updates**: Click one button to update all projects
5. **Audit Trail**: All calculations logged for verification

## Best Practices

### When to Calculate
- ✅ After entering/updating weekly attendance
- ✅ Before generating project reports
- ✅ When reviewing project costs
- ✅ At end of pay period
- ✅ Before project completion

### Regular Schedule
Recommended: Calculate payroll costs weekly after attendance entry

## Logging

### Success Messages
```
Updated payroll for Employee 5 in project PRJ-00001: 160.5 hours, ₱8025.00
Updated payroll costs for 3 project(s)
```

### Warning Messages
```
Project expense database not found for PRJ-00001
No projects found or no payroll costs to calculate
```

### Info Messages
```
Project payroll costs calculated successfully
```

## Troubleshooting

### Issue: No costs calculated
**Cause:** No employees assigned to projects  
**Solution:** Assign employees by setting their site location to a project ID

### Issue: Costs seem incorrect
**Cause:** Old attendance data or incorrect hourly rate  
**Solution:** 
1. Verify employee hourly rate in Employee Management
2. Check weekly attendance records
3. Recalculate payroll costs

### Issue: Project not updated
**Cause:** Project expense database doesn't exist  
**Solution:** Ensure project was created in Project Management

## Future Enhancements

Potential improvements:
- **Automatic Scheduling**: Calculate costs automatically after attendance entry
- **Date Range Filtering**: Calculate costs for specific date ranges
- **Cost Breakdown**: Show per-week cost breakdown
- **Budget Alerts**: Warn when labor costs exceed project budget
- **Historical Tracking**: Track cost changes over time

## Files Modified

- `src/core/monitor.h` - Function declarations
- `src/core/monitor.cpp` - Implementation with SQLite queries
- `src/ui/ui.cpp` - UI button and event handling
- `doc/SITE_LOCATION_DROPDOWN_AND_PROJECT_PAYROLL.md` - Updated documentation

## See Also

- `doc/SITE_LOCATION_DROPDOWN_AND_PROJECT_PAYROLL.md` - Overall payroll tracking system
- `doc/LOAD_EMPLOYEE_DATA_FEATURE.md` - Employee data loading
- `CHANGELOG.md` - Version history and changes

