## What's Changed

#### Windows Release Build Configuration Fix (December 18, 2024) - COMPREHENSIVE UPDATE
- **Completely eliminated Debug runtime library dependencies in Release builds**
  - ✅ Set `CMAKE_MSVC_RUNTIME_LIBRARY` globally **before** project() to apply to all dependencies
  - ✅ Explicitly removed all `/MDd`, `/MTd`, and `_DEBUG` flags from Release configurations
  - ✅ Force `/MD` and `NDEBUG` flags for all Release build types
  - ✅ Applied runtime library settings to all FetchContent dependencies (including hello_imgui)
  - ✅ Double-enforced runtime settings on main target for extra safety
  
- **Build Verification System**
  - ✅ Added automated verification scripts to detect debug DLLs
  - ✅ GitHub Actions now **fails the build** if debug runtime libraries are detected
  - ✅ Created `verify_release_build.ps1` and `verify_release_build.bat` for manual verification
  - ✅ Uses `dumpbin` to inspect executable dependencies post-build
  
- **Multi-Layer Protection**
  1. **Pre-project configuration** - Sets MSVC runtime library before any targets
  2. **Global compiler flags** - Removes debug flags, forces release flags
  3. **Dependency configuration** - Ensures FetchContent uses correct runtime
  4. **Target-specific enforcement** - Double-checks main executable settings
  5. **Post-build verification** - Validates final executable doesn't link debug DLLs
  
- **What This Fixes**
  - ❌ **Before:** `ucrtbased.dll`, `vcruntime140d.dll` → Error 0xc000007b
  - ✅ **After:** `ucrtbase.dll`, `vcruntime140.dll` → Works with VC++ Redistributable

#### Windows Error 0xc000007b Fix - AUTOMATIC INSTALLATION
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
