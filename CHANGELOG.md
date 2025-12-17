## What's Changed

### Automatic Attendance Synchronization
- **New employees automatically prepared for attendance tracking** when added to base_payroll.db
- **Employee deletion automatically removes all attendance records** across all weekly databases
- **Employee ID updates automatically propagate** to all attendance records in both base_payroll.db and weekly databases
- Scans all 52 weekly attendance databases (12 months) to ensure complete synchronization
- Provides detailed logging of attendance record updates and deletions
- Ensures data integrity between employee records and attendance tracking

### Bug Fixes
- Fixed ImGui "Missing EndGroup()" error in Generate Reports section
- Removed leftover BeginChild/EndChild containers from UI refactoring
- Corrected ImGui element pairing for proper UI rendering
- **Fixed payslip generation to fetch data from database**: All payslip data now properly retrieved from WEEKLY_ATTENDANCE table
- **Fixed missing weekly hours table in payslips**: Table now displays when attendance data exists in database
- **Implemented duplicate log message detection**: Repeated log messages are combined into single line with count (e.g., "message (x5)")
- **CRITICAL: Fixed employee ID data type mismatch** between EMPLOYEES (INTEGER) and WEEKLY_ATTENDANCE (now INTEGER)
- **Ensured payroll computation properly links** base payroll and weekly attendance via matching INTEGER employee IDs
- Employee IDs now stored as numeric values (1, 2, 3...) in both tables for proper database joins
- **Fixed payslip distortion**: Reduced padding, margins, and font sizes to fit all content (including weekly hours) in 2x4 grid layout without overflow

### Payslip Weekly Hours Table
- **Weekly hours section now ALWAYS displays**, even when hours are 0 or no attendance data exists
- Daily hours shown as compact inline badges: `Sun: 0.0  Mon: 0.0  Tue: 0.0` etc. (shows 0.0 when no data)
- Total hours highlighted with orange background matching payslip theme
- Display appears in light orange box with rounded corners
- **Hours worked ONLY from weekly attendance database** - base payroll hours are ignored
- **Employee IDs use numeric format only** (1, 2, 3...) matching base_payroll.db INTEGER PRIMARY KEY
- Fetches most recent week's attendance data for each employee
- Shows detailed daily work hours (Sunday through Saturday)
- **Critical:** All payslip calculations based solely on attendance data, NOT base payroll HOURS_WORK field

### Input Validation System
- Implemented comprehensive input validation system with multiple validation types (NAME, POSITION, SALARY, HOURS, ADVANCE, DATE_FORMAT, EMPLOYEE_ID, PROJECT_ID_FORMAT, QUANTITY, MATERIAL_ID)
- Added real-time validation with visual feedback - invalid inputs display with bright red background (`ImVec4(0.6f, 0.15f, 0.15f, 1.0f)`)
- Applied validation to all input fields across Employee Management, Project Management, and Materials & Expenses sections
- Validation rules:
  - Names and positions: Letters and spaces only (max 100/50 chars)
  - Salary/prices: Positive decimal numbers only
  - Hours: Decimal numbers in 0-168 range
  - Advance: Non-negative decimal numbers (can be empty)
  - Dates: Strict ISO 8601 format (YYYY-MM-DD) with date range validation
  - Employee IDs: Positive integers only
  - Project IDs: PRJ-##### format

### Year-Based Attendance Directory System
- Implemented automatic year-based attendance directory organization
- Attendance databases now stored in `data/payroll/attendance/{YEAR}/` structure
- Directory path automatically updates with current year at application startup
- Uses lambda function initialization in config.cpp for dynamic year inclusion
- Weekly attendance databases follow format: `MM-DD-DD.db` within year folder

### UI Improvements
- Reorganized "Generate Reports" section in Summary UI with horizontal layout
- Payslip Reports: All controls in single row (Print All button, week dropdown, Print Week button)
- **Replaced text input with dropdown for week selection** in payslip generation (matches weekly attendance UI)
- Week dropdown shows all Sundays in current year (e.g., "01/05-11", "01/12-18")
- Project Reports: All controls in single row (Project ID input, Print Report button)
- Removed child containers for cleaner, more compact design
- Added validation for Employee ID in Weekly Attendance Management with red background indicator
- Improved spacing and alignment throughout the UI

### Report Generation Fixes
- Fixed duplicate browser tabs opening when generating reports
- Removed redundant `system::openFileInBrowser()` calls from UI layer
- Print functions (`exportPayslipsHtml`, `exportPayslipsHtmlForWeek`, `exportProjectReportHtml`) now handle browser opening internally
- Only one browser tab now opens per report generation

### Application Shutdown Enhancement
- Enhanced `appShutdown()` function with comprehensive error handling
- Added try-catch blocks around database encryption, backup cleanup, and backup creation operations
- Improved graceful shutdown to prevent "unexpected close" system warnings
- Application now exits cleanly even if backup operations fail
- Detailed error logging for shutdown operations
- Removed `exit(0)` call - application returns normally from `main()`

### Configuration Updates
- Added `g_assetsDirectory` to application configuration
- Added `g_payrollAttendanceDirectory` with year-based initialization
- Improved config initialization with automatic year-based path setup

### Code Quality
- Refactored monitor.cpp to use config-based attendance directory paths
- Removed hardcoded year paths throughout the codebase
- Enhanced error messages and logging for better debugging
- Improved validation logic consistency across all UI components


## Supported Platforms

### Linux
- **Debian/Ubuntu**: Ubuntu 22.04, 20.04, Debian 12 (x64, ARM64)
- **Red Hat/Fedora**: Fedora Latest, AlmaLinux 9, Rocky Linux 9 (x64, ARM64)
- **SUSE**: openSUSE Tumbleweed, Leap 15.6 (x64, ARM64)

### macOS
- Apple Silicon (ARM64) - macOS 11.0+
- Intel (x64) - macOS 15.0+

### Windows
- x64 and ARM64


## Build Information
- Build Type: Pre-Release
- Optimizations: Enabled (LTO)
- Compiler Flags: -O3 (Linux/macOS), /O2 (Windows)

## Requirements

### Linux
All builds require standard system libraries:
```bash
# Debian/Ubuntu
sudo apt-get install libfreetype6 libgl1 libx11-6 libxrandr2 libxinerama1 libxcursor1 libxi6


# Fedora/Red Hat
sudo dnf install freetype mesa-libGL libX11 libXrandr libXinerama libXcursor libXi

# SUSE
sudo zypper install freetype2 Mesa-libGL libX11-6 libXrandr2 libXinerama1 libXcursor1 libXi6
```

### macOS
You need to execute this in the terminal before launching the app as macOS shows this error because the app is not code-signed.
macOS Gatekeeper blocks unsigned apps downloaded from the internet by setting a "quarantine" attribute.
```bash
xattr -cr <location-of-the-.app>
```

### Windows
No additional dependencies required.

## Support

For issues, questions, or feature requests, please visit the [Issues](
https://github.com/joshua-9402/cpet-140-final-project/issues) page.

## Documentation

See the [README](README.md) for full documentation and usage instructions.
