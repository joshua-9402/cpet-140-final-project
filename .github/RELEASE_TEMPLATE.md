## What's Changed

- change name from "payroll-and-monitoring-system" to "structuracost" in build files and documentations. See commit [`75a4bab`](https://github.com/joshua-9402/cpet-140-final-project/commit/75a4bab0f036a18081a3da60fd507748d9ccc84e)
- remove support for legacy macOS versions. See commit [`8277da5`](https://github.com/joshua-9402/cpet-140-final-project/commit/8277da5d852244ce8b1bc5b2b6507f01b169e7d3)
- change the asset naming scheme to include platform and architecture clearly. See commit [`8206b50`](https://github.com/joshua-9402/cpet-140-final-project/commit/8206b5068d02c7fa9c681b1eaf16be0c1d8c0cc4)

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
tar -xzf structuracost-*.tar.gz
./structuracost
```

**Windows:**
```powershell
Expand-Archive structuracost-*.zip
.\structuracost.exe
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

