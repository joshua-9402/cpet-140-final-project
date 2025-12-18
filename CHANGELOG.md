## What's Changed

#### Windows Error 0xc000007b Fix - AUTOMATIC INSTALLATION
- **Windows installer now AUTOMATICALLY installs all required dependencies**
  - ✅ **Visual C++ Redistributable 2015-2022 (x64)** - Auto-downloaded and installed
  - ✅ **Visual C++ Redistributable 2015-2022 (x86)** - Auto-downloaded and installed  
  - ✅ **libsodium.dll** - Auto-downloaded and installed (correct architecture)
  - Installer shows progress window during dependency installation
  - No manual intervention required - just run the installer!
  
- **Enhanced installer features:**
  - Post-install dependency check and installation
  - Automatic detection of already-installed components
  - Graceful fallback with manual installation instructions
  - Bundled troubleshooting documentation
  
- **Build system improvements:**
  - Added proper MSVC runtime library configuration (MultiThreadedDLL)
  - Ensured architecture consistency (x64/Win32) throughout build
  - Automatic DLL copying for correct architecture
  - Post-build checks to verify required dependencies


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
