## What's Changed

#### Site Location Dropdown with Dynamic Project List
- **Replaced text input with dropdown** for Site Location in Employee Management
  - Fixed options: "Main Office" and "Warehouse"
  - Dynamic options: All project IDs from database (auto-refreshes every 60 frames)
  - Prevents typos and ensures data consistency
  - **New Validation**: Added `SITE_LOCATION` validation type that accepts:
    - "Main Office"
    - "Warehouse"  
    - "PRJ-XXXXX" format (where X is any digit)

#### Automatic Project Payroll Expense Tracking
- **New Feature**: Employees assigned to projects are automatically added to project payroll expenses
  - Added `PAYROLL_EXPENSES` table to project expense databases (PRJ-xxxxx.db)
  - Tracks: Employee ID, Name, Position, Hourly Rate, Total Hours, Total Cost
  - Smart location change handling:
    - Moving employee from project A to project B automatically updates both projects
    - Deleting an employee removes them from any assigned project
  - **Automatic Payroll Calculation**: Calculate project labor costs from weekly attendance
    - Button: "Calculate All Project Payroll Costs" in Monitor UI
    - **Date-Filtered**: Only counts hours between project START_DATE and END_DATE
    - Supports open-ended projects (no END_DATE = counts from START_DATE to present)
    - Calculates total cost: `Total Hours × Hourly Rate`
    - Updates `TOTAL_HOURS` and `TOTAL_COST` in project's PAYROLL_EXPENSES table
  - **Project Report Integration**: Payroll expenses shown SEPARATELY in project reports
    - Dedicated "Payroll Expenses" section in HTML reports
    - Shows employee details: ID, Name, Position, Hourly Rate, Total Hours, Total Cost
    - Summary breakdown: "Total Materials" + "Total Payroll" = "PROJECT TOTAL COST"
  - Files modified: `src/handler/db.h`, `src/handler/db.cpp`, `src/core/monitor.h`, `src/core/monitor.cpp`, `src/ui/ui.cpp`, `src/handler/print.cpp`
  - New functions: 
    - `db::insertPayrollExpense()`, `db::updatePayrollExpense()`, `db::deletePayrollExpense()`
    - `monitor::calculateProjectPayrollCosts()`, `monitor::calculateAllProjectPayrollCosts()`
    - `print::fetchProjectPayroll()`

#### Load Data Buttons  
- **New Feature**: "Load" buttons added to management sections
  - **Employee Management**: "Load Employee" button
    - Fetches and populates all employee fields by ID
    - Syncs site location dropdown
  - **Project Management**: "Load Project" button
    - Fetches and populates all project fields by ID
    - Syncs status dropdown
  - All buttons: 260px width, 40px height, same row as other action buttons

#### Project End Date Tracking
- **New Feature**: Added END_DATE field to PROJECT_LIST table
  - Optional field for tracking project completion
  - Used for date-filtering payroll cost calculations
  - UI: End Date input in Project Management (validates YYYY-MM-DD)
  - Empty END_DATE = ongoing project (counts hours from START_DATE to present)
  - Files: `src/handler/db.cpp`, `src/core/monitor.cpp`, `src/ui/ui.cpp`

#### Load Employee Data Button
- **New Feature**: "Load Employee" button in Employee Management action row
  - Fetches employee data from database by Employee ID
  - Autopopulates all input fields (Name, Position, Site Location, etc.)
  - Syncs site location dropdown to match loaded employee's location
  - Streamlines employee update workflow - no need to retype existing data
  - Button placement: Same row as Add/Update/Delete buttons (4th button, 260px width)

#### Logging System Improvements
- **Converted `std::cerr` to `system::logMessage()`** in print module
  - All error messages now use centralized logging system
  - Maintains log files instead of ephemeral console output
  - Better error tracking and debugging capability

#### Documentation Updates
- Added `doc/SITE_LOCATION_DROPDOWN_AND_PROJECT_PAYROLL.md` - Comprehensive guide for site location dropdown and project payroll features
- Added `doc/LOAD_EMPLOYEE_DATA_FEATURE.md` - Usage guide for Load Employee button
- Updated existing documentation to reflect new features

### Cross-platform libsodium auto-install & installer improvements
- Implemented automatic libsodium installation for all platforms:
  - Linux: `packaging/linux/postinst.sh` (post-install script hooked into .deb/.rpm)
  - Windows: `packaging/windows/install_libsodium.ps1` (PowerShell) and NSIS integration
  - macOS: `packaging/macos/postinstall` (postinstall script for .pkg)
- Made `assets/icons/app_icon.png` the default application icon (window/dock/taskbar) and added a macOS helper to set Dock icon at runtime.
- Added NSIS installer commands to create a desktop shortcut on Windows.
- Bundled logic in `CMakeLists.txt` to copy the icon asset, include platform-specific helper sources, and include the post-install scripts in generated packages.


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
