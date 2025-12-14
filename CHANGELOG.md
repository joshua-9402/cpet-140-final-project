## What's Changed

### 🎉 Major Features & Enhancements

#### Print & Report Generation
- Added comprehensive project report generation with HTML export to browser
- Introduced HTML payslip generator with 2x4 layout options and improved margins
- Implemented project and material structures with database fetching capabilities
- Enhanced `print.cpp` and `print.h` with live data integration matching payslip styling

#### UI Improvements
- Enhanced UI with project report printing and database viewer features (`ui.cpp`)
- Implemented payroll computation functions and enhanced payroll UI
- Simplified login UI by removing unused padding and error handling code
- Renamed `basicAuth` to `mainAuth` for better clarity
- Enhanced passkey hashing functionality in UI components

#### Monitoring System
- Extended monitoring tools (`monitor.cpp/.h`) with richer project management and expense tracking
- Added worker and equipment monitoring functions with cost calculations
- Implemented background thread for periodic employee ID monitoring and rearrangement (`main.cpp`)

#### Database & Data Management
- Updated database schema and added `fetchCell` function for improved data retrieval
- Improved input validation and simplified SQL query preparation in `db.cpp`
- Removed obsolete materials database references for cleaner architecture
- Added algorithm header for improved functionality

#### Payroll System
- Refined payroll modules (`payroll.cpp/.h`) with attendance helpers and standardized naming
- Standardized class naming and added attendance record functions
- Removed unnecessary whitespace and comments for code cleanliness

#### System & Core
- Updated `system.cpp/.h` to expose project report workflows and ensure logout returns to the login UI
- Added project report generation function in system module

### 🔒 Security & Authentication
- Updated authentication methods to include standard and admin users
- Renamed `basicAuth` to `mainAuth` throughout the codebase for consistency
- Enhanced username verification in `auth.cpp`
- Removed deprecated admin authentication method

### 🛠️ Code Quality & Refactoring
- Streamlined hash computation by combining variable declaration and condition check in `cryptography.cpp`
- Corrected typo in boundaries section and removed unused includes in `monitor.h`
- Improved code maintainability across multiple modules

### 🖥️ Platform Support
- Switch from macOS 13 to macOS 15 deployment target for Intel builds
- Updated GitHub Actions workflow to use `macos-15` runner for Intel Macs
- Enhanced cross-platform compatibility

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

For issues, questions, or feature requests, please visit the [Issues](https://github.com/joshua-9402/cpet-140-final-project/issues) page.

## Documentation

See the [README](../README.md) for full documentation and usage instructions.
