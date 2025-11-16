# libsodium Setup Guide

This document explains how libsodium is integrated into the project and how to set it up on different platforms and architectures.

## Overview

The CMakeLists.txt uses a **multi-strategy approach** to find libsodium, trying multiple methods in order:

1. **pkg-config** (Linux/Unix systems)
2. **Homebrew** (macOS - both Apple Silicon and Intel)
3. **Local dependencies folder** (all platforms)
4. **Standard system paths** (Linux)

## Platform-Specific Setup

### macOS (Apple Silicon & Intel)

**Easiest method - Install via Homebrew:**
```bash
brew install libsodium
```

The build system will automatically find libsodium at:
- Apple Silicon (M1/M2/M3): `/opt/homebrew/lib/libsodium.dylib`
- Intel Mac: `/usr/local/lib/libsodium.dylib`

**Alternative - Manual installation:**
1. Download from: https://github.com/jedisct1/libsodium/releases
2. Place files in `dependencies/libsodium/`:
   ```
   dependencies/libsodium/
   ├── include/
   │   └── sodium.h
   └── lib/
       ├── libsodium.dylib  (preferred)
       └── libsodium.a      (alternative)
   ```

### Linux (x86_64 & ARM64)

**Debian/Ubuntu:**
```bash
sudo apt-get install libsodium-dev
```

**Fedora/RHEL/CentOS:**
```bash
sudo dnf install libsodium-devel
```

**SUSE/openSUSE:**
```bash
sudo zypper install libsodium-devel
```

**Arch Linux:**
```bash
sudo pacman -S libsodium
```

**Manual installation (if package not available):**
1. Download from: https://github.com/jedisct1/libsodium/releases
2. Place files in `dependencies/libsodium/`:
   ```
   dependencies/libsodium/
   ├── include/
   │   └── sodium.h
   └── lib/
       ├── libsodium.so  (preferred)
       └── libsodium.a   (alternative)
   ```

### Windows (x64 & ARM64)

**Manual installation required:**
1. Download prebuilt binaries from: https://github.com/jedisct1/libsodium/releases
   - For x64: Download `libsodium-*-msvc.zip`
   - For ARM64: Download ARM64-compatible build
2. Extract and place files in `dependencies/libsodium/`:
   ```
   dependencies/libsodium/
   ├── include/
   │   └── sodium.h
   └── lib/
       ├── libsodium.lib  (import library)
       └── libsodium.dll  (runtime - for deployment)
   ```

## Supported Architectures

The build system supports all common architectures:

### macOS
- ✅ Apple Silicon (arm64) - M1/M2/M3
- ✅ Intel (x86_64)

### Linux
- ✅ x86_64 (AMD64)
- ✅ ARM64 (aarch64)
- ✅ ARM32 (armhf) - with system libraries

### Windows
- ✅ x64 (AMD64)
- ✅ ARM64 (experimental)

## Directory Structure

The build system checks for libsodium in multiple locations:

### Local dependencies (cross-platform):
```
dependencies/libsodium/
├── include/
│   └── sodium.h
└── lib/          # or unix/ or windows/
    └── libsodium.[a|so|dylib|lib]
```

### Homebrew paths (macOS):
- `/opt/homebrew/` (Apple Silicon)
- `/usr/local/` (Intel)

### System paths (Linux):
- `/usr/lib/`
- `/usr/local/lib/`
- `/usr/lib/x86_64-linux-gnu/` (Debian/Ubuntu x64)
- `/usr/lib/aarch64-linux-gnu/` (Debian/Ubuntu ARM64)

## Troubleshooting

### Build fails with "libsodium not found"

1. **Check if installed:**
   - macOS: `brew list libsodium`
   - Linux: `dpkg -l | grep libsodium` or `rpm -qa | grep libsodium`
   - Windows: Check `dependencies/libsodium/` folder

2. **Install using package manager** (recommended):
   - Follow platform-specific instructions above

3. **Manual installation**:
   - Download from https://github.com/jedisct1/libsodium/releases
   - Place in `dependencies/libsodium/` with correct structure
   - Ensure files have correct permissions

### CMake can't find libsodium even though it's installed

1. **Clear CMake cache:**
   ```bash
   rm -rf cmake-build-debug
   mkdir cmake-build-debug
   cd cmake-build-debug
   cmake ..
   ```

2. **Check PKG_CONFIG_PATH** (Linux):
   ```bash
   export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
   ```

3. **Verify installation paths:**
   - macOS: `ls -la /opt/homebrew/lib/libsodium.*`
   - Linux: `ldconfig -p | grep libsodium`

### Different architecture on same machine

If you need to build for different architectures on the same machine:

**macOS Universal Binary:**
```bash
cmake -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" ..
```

**Linux cross-compilation:**
```bash
# Install cross-compiler and libsodium for target arch
sudo apt-get install gcc-aarch64-linux-gnu libsodium-dev:arm64
cmake -DCMAKE_TOOLCHAIN_FILE=... ..
```

## CI/CD Integration

The multi-strategy approach ensures builds work in CI/CD environments:

### GitHub Actions (example)
```yaml
- name: Install libsodium
  run: |
    if [[ "$RUNNER_OS" == "Linux" ]]; then
      sudo apt-get update
      sudo apt-get install -y libsodium-dev
    elif [[ "$RUNNER_OS" == "macOS" ]]; then
      brew install libsodium
    elif [[ "$RUNNER_OS" == "Windows" ]]; then
      # Download and extract to dependencies/libsodium
      curl -L -o libsodium.zip https://download.libsodium.org/libsodium/releases/libsodium-*-msvc.zip
      unzip libsodium.zip -d dependencies/libsodium
    fi
```

## Building from Source (Advanced)

If you need to build libsodium from source:

```bash
# Clone repository
git clone https://github.com/jedisct1/libsodium.git
cd libsodium
git checkout stable

# Build
./autogen.sh
./configure --prefix=$PWD/build-output
make
make install

# Copy to project
cp -r build-output/lib/* /path/to/project/dependencies/libsodium/lib/
cp -r build-output/include/* /path/to/project/dependencies/libsodium/include/
```

## Version Requirements

- **Minimum version**: libsodium 1.0.18+
- **Recommended**: libsodium 1.0.20 (latest stable)
- **Compatibility**: All versions since 1.0.18 should work

## Security Notes

- Always use the latest stable version of libsodium
- Verify checksums when downloading binaries
- Keep libsodium updated for security patches
- Use package managers when possible (automatic updates)

## Additional Resources

- Official website: https://libsodium.gitbook.io/
- GitHub repository: https://github.com/jedisct1/libsodium
- Documentation: https://doc.libsodium.org/
- Installation guide: https://libsodium.gitbook.io/doc/installation

