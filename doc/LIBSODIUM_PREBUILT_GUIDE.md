# Libsodium Prebuilt Libraries Setup Guide

This guide explains how to add prebuilt libsodium libraries to the project for distribution.

## Overview

The CMake build system automatically detects and bundles prebuilt libsodium libraries placed in the `dependencies/libsodium` directory. These libraries will be included in installers and packages for all platforms.

## Directory Structure

Place prebuilt libsodium files in the following structure:

```
dependencies/
└── libsodium/
    ├── include/
    │   └── sodium.h
    │   └── sodium/
    │       └── ... (other headers)
    │
    ├── Windows (x64)/
    │   ├── x64/
    │   │   └── Debug/ or Release/
    │   │       ├── libsodium.lib
    │   │       └── libsodium.dll
    │   └── ARM64/
    │       └── ... (similar structure)
    │
    ├── macOS/
    │   └── lib/
    │       ├── libsodium.dylib
    │       └── libsodium.a
    │
    └── Linux/
        └── lib/
            └── libsodium.so
```

## Platform-Specific Instructions

### Windows

1. Download prebuilt libsodium from: https://download.libsodium.org/libsodium/releases/
2. Extract and place files under `dependencies/libsodium/`
3. Ensure the directory structure includes:
   - `include/sodium.h`
   - `x64/*/libsodium.lib` (import library)
   - `x64/*/libsodium.dll` (runtime)
4. The CMake script will:
   - Auto-detect your target architecture (x64 or ARM64)
   - Link the appropriate .lib file
   - Bundle the .dll in the installer

### macOS

1. Build libsodium from source or use Homebrew
2. Copy to `dependencies/libsodium/`:
   - `include/` directory with headers
   - `lib/libsodium.dylib` or `lib/libsodium.a`
3. The CMake script will:
   - Link the library
   - Bundle dylib into the .app/Contents/Frameworks

### Linux

#### Note about Linux packaging

Linux packaging (.deb/.rpm) is no longer produced by this repository's release pipeline. For Linux packaging guidance, see `doc/LINUX_PACKAGING_REMOVED.md`.

## Build Verification

After placing the libraries, reconfigure CMake:

```bash
cmake -S . -B build
cmake --build build
```

Check the configuration output for libsodium detection messages.

## Packaging

When creating installers/packages on supported platforms (Windows/macOS):

```bash
cd build
cpack
```

The generated installers will automatically include:
- The application executable
- Any required bundled runtime libraries
- Application icon
- Desktop shortcuts (where applicable)

## Notes

- **Linux**: Automatically downloads and installs libsodium during package installation if not present
- **Windows/macOS**: Requires prebuilt libraries in `dependencies/libsodium/` before packaging
- If prebuilt libraries are not present on Windows/macOS, CMake will search system paths (Homebrew, vcpkg, etc.)
- Windows installers will create desktop shortcuts with the app icon
- macOS bundles will use app_icon.png as the dock icon
- All platforms will display app_icon.png as the window icon
