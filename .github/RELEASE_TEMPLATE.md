## What's Changed

This the first pre-release of the Payroll and Monitoring System application, StructuraCost.

This release bundles optimized, prebuilt artifacts for desktop platforms: native C/C++ binaries (x64 and ARM64) for Linux / macOS / Windows

## Supported Platforms

### Linux
- **Debian/Ubuntu**: Ubuntu 22.04, 20.04, Debian 12 (x64, ARM64)
- **Red Hat/Fedora**: Fedora Latest, AlmaLinux 9, Rocky Linux 9 (x64, ARM64)
- **SUSE**: openSUSE Tumbleweed, Leap 15.6 (x64, ARM64)

### macOS
- Apple Silicon (ARM64) - macOS 14.0+
- Intel (x64) - macOS 11.0+

### Windows
- x64 and ARM64

## Installation

Download the appropriate archive for your platform and extract it.

**Linux/macOS:**
```bash
tar -xzf payroll-and-monitoring-system-*.tar.gz
./payroll-and-monitoring-system
```

**Windows:**
```powershell
Expand-Archive payroll-and-monitoring-system-*.zip
.\payroll-and-monitoring-system.exe
```

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
No additional dependencies required.

### Windows
No additional dependencies required.

## Support

For issues, questions, or feature requests, please visit the [Issues](https://github.com/joshua-9402/cpet-140-final-project/issues) page.

## Documentation

See the [README](../README.md) for full documentation and usage instructions.

