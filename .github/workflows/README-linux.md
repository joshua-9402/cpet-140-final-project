# Linux Build Workflows (Debian-based Distributions)

## Overview

This workflow builds the payroll-and-monitoring-system for **Debian-based Linux distributions** across **x64 and ARM64 architectures**.

## Supported Configurations

### Debian-based Distributions
- **Ubuntu 22.04** (Jammy Jellyfish - Latest LTS)
- **Ubuntu 20.04** (Focal Fossa - Previous LTS)
- **Ubuntu Latest** (Rolling)
- **Debian 12** (Bookworm - Latest Stable)
- **Debian 11** (Bullseye - Oldstable)
- **Debian Latest** (Current Stable)

### Derivative Distributions (Should also work)
These Debian/Ubuntu derivatives should also work with the builds:
- **Linux Mint** (based on Ubuntu)
- **Pop!_OS** (based on Ubuntu)
- **elementary OS** (based on Ubuntu)
- **Zorin OS** (based on Ubuntu)
- **KDE neon** (based on Ubuntu LTS)
- **Raspberry Pi OS** (based on Debian)
- **MX Linux** (based on Debian)
- **Linux Lite** (based on Ubuntu)
- **Peppermint OS** (based on Debian)

### Architectures
- **x64** (AMD64/Intel 64-bit)
- **ARM64** (AArch64/ARM 64-bit)

### Total Combinations
**6 distributions × 2 architectures = 12 possible builds**

**Plus compatibility with numerous Debian/Ubuntu derivatives!**

## Key Features

- ✅ **Manual trigger only** - Build when you need it
- ✅ **Multi-distribution support** - Ubuntu, Fedora, Debian, Arch
- ✅ **Multi-architecture** - x64 and ARM64
- ✅ **Container-based builds** - Consistent environment
- ✅ **Configurable** - Choose Release or Debug
- ✅ **Optional LTO** - Enable/disable Link-Time Optimization
- ✅ **90-day retention** - Long artifact storage

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Build Linux (Manual)"** from the workflow list
4. Click the **"Run workflow"** button
5. Configure options:
   - **Distribution:** Choose your Debian-based Linux distribution
     - `ubuntu-latest` - Latest Ubuntu
     - `ubuntu-22.04` - Ubuntu 22.04 LTS (Jammy)
     - `ubuntu-20.04` - Ubuntu 20.04 LTS (Focal)
     - `debian-latest` - Debian Stable (Current)
     - `debian-12` - Debian 12 (Bookworm)
     - `debian-11` - Debian 11 (Bullseye)
   - **Architecture:** Choose `x64` or `arm64`
   - **Build type:** Choose `Release` or `Debug`
   - **Enable LTO:** Check for maximum performance
6. Click **"Run workflow"**

### From GitHub CLI

```bash
# Ubuntu 22.04 x64 Release
gh workflow run "Build Linux (Manual)" \
  --ref master \
  -f distribution=ubuntu-22.04 \
  -f architecture=x64 \
  -f build_type=Release \
  -f enable_lto=true

# Debian 12 ARM64 Release
gh workflow run "Build Linux (Manual)" \
  --ref master \
  -f distribution=debian-12 \
  -f architecture=arm64 \
  -f build_type=Release \
  -f enable_lto=true

# Ubuntu 20.04 x64 Debug
gh workflow run "Build Linux (Manual)" \
  --ref master \
  -f distribution=ubuntu-20.04 \
  -f architecture=x64 \
  -f build_type=Debug \
  -f enable_lto=false
```

## Distribution Details

### Ubuntu (Debian-based)

**Package Manager:** apt  
**Compiler:** GCC (default version per release)  
**Best For:** Most users, LTS stability, wide hardware support

**Versions:**
- `ubuntu-latest` - Rolling latest (currently 24.04)
- `ubuntu-22.04` - Jammy Jellyfish (LTS until 2027)
- `ubuntu-20.04` - Focal Fossa (LTS until 2025)

**Dependencies Installed:**
- build-essential, cmake, ninja-build
- X11/OpenGL development libraries
- FreeType and Fontconfig

### Debian (Pure Debian)

**Package Manager:** apt  
**Compiler:** GCC (stable version)  
**Best For:** Maximum stability, server deployments, Raspberry Pi

**Versions:**
- `debian-latest` - Current Debian Stable
- `debian-12` - Bookworm (Latest stable)
- `debian-11` - Bullseye (Oldstable)

**Dependencies Installed:**
- Same as Ubuntu (both use apt and Debian packages)

### Why Debian-based Only?

**Consistency:**
- All use `apt` package manager
- Same package names across distributions
- Compatible binary format
- Shared library ecosystem

**Coverage:**
- Ubuntu is the most popular Linux desktop distribution
- Debian is widely used for servers
- Most Debian/Ubuntu derivatives will work with these builds

**Simplicity:**
- One set of dependencies
- Predictable build environment
- Easier maintenance

## Architecture Details

### x64 (AMD64/Intel 64-bit)

**Also known as:** amd64, x86_64, x86-64  
**Compatibility:** Intel and AMD processors  
**Runner:** Ubuntu latest (x64)  
**Most common:** Desktop and server systems

### ARM64 (AArch64)

**Also known as:** aarch64, ARM 64-bit  
**Compatibility:** ARM-based systems  
**Runner:** Ubuntu 22.04 ARM64  
**Common on:** Raspberry Pi 4/5, ARM servers, cloud instances

## Build Configuration

### Build Types

**Release** (Recommended for distribution)
- Maximum optimization (`-O3`)
- Link-Time Optimization (if enabled)
- No debug symbols
- Smaller binary size
- Fastest performance

**Debug** (For development)
- No optimization (`-O0`)
- Debug symbols (`-g`)
- Larger binary size
- Better debugging with GDB

### Link-Time Optimization (LTO)

**Enabled (Default):**
- Whole-program optimization
- Better performance (~5-10%)
- Longer build time
- Flags: `-O3 -flto -DNDEBUG`

**Disabled:**
- Standard optimization
- Faster build time
- Flags: `-O3 -DNDEBUG`

## Container-Based Builds

This workflow uses Docker containers for consistent builds:

```yaml
container:
  image: ubuntu:22.04  # Or fedora:latest, debian:latest, archlinux:latest
```

**Benefits:**
- ✅ Clean, reproducible environment
- ✅ Matches target distribution exactly
- ✅ Isolated from runner OS

## What Gets Built

The workflow produces:
- Native Linux executable (ELF binary)
- Includes all assets (fonts, icons)
- Packaged as `.tar.gz` archive
- Distribution and architecture specific
- Dependencies: Dynamically linked (requires system libraries)

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section
3. Download: `payroll-and-monitoring-system-ubuntu-22.04-x64-Release.tar.gz`

**Retention:** 90 days

### Extract and Run

```bash
# Extract
tar -xzf payroll-and-monitoring-system-ubuntu-22.04-x64-Release.tar.gz

# Run
./payroll-and-monitoring-system
```

### Check Dependencies

```bash
# See required libraries
ldd payroll-and-monitoring-system

# Common dependencies:
# - libfreetype.so.6
# - libGL.so.1
# - libX11.so.6
# - libstdc++.so.6
# - libgcc_s.so.1
```

## Compatibility Matrix

| Distribution | Architecture | Tested On | Likely Compatible With |
|--------------|--------------|-----------|------------------------|
| Ubuntu 22.04 | x64 | Ubuntu 22.04 | Ubuntu 22.04+, Mint 21+, Pop!_OS 22.04+, elementary OS 7+ |
| Ubuntu 22.04 | arm64 | Ubuntu 22.04 ARM64 | Raspberry Pi OS 64-bit, Ubuntu ARM |
| Ubuntu 20.04 | x64 | Ubuntu 20.04 | Ubuntu 20.04+, Mint 20+, elementary OS 6+ |
| Ubuntu 20.04 | arm64 | Ubuntu 20.04 ARM64 | Raspberry Pi OS, Ubuntu ARM |
| Debian 12 | x64 | Debian 12 | Debian 12+, MX Linux 23+, antiX |
| Debian 12 | arm64 | Debian 12 ARM64 | Raspberry Pi OS (Bookworm), Debian ARM |
| Debian 11 | x64 | Debian 11 | Debian 11+, MX Linux 21+ |
| Debian 11 | arm64 | Debian 11 ARM64 | Raspberry Pi OS (Bullseye) |

## System Requirements (For Running)

**All Distributions:**
- OpenGL support (hardware or software)
- X11 display server
- Required shared libraries (installed via package manager)

**Installing Dependencies (End User):**

**Ubuntu/Debian and derivatives:**
```bash
sudo apt-get install libfreetype6 libgl1 libx11-6 libxrandr2 libxinerama1 libxcursor1 libxi6
```


## Build Time

Typical build times:

| Configuration | x64 Time | ARM64 Time |
|--------------|----------|------------|
| Release + LTO | 8-12 min | 12-18 min |
| Release (no LTO) | 5-8 min | 8-12 min |
| Debug | 4-6 min | 6-10 min |

**Note:** ARM64 builds are slower due to cross-compilation or emulation.

## Troubleshooting

### Build Failed

**Container pull issues:**
- Check Docker Hub status
- Verify distribution name is correct

**Dependency installation failed:**
- Package names may differ between distributions
- Check workflow logs for specific package errors

**Compilation errors:**
- Check GCC version compatibility
- Verify C++23 support (GCC 11+ required)

### Binary Won't Run

**Missing libraries:**
```bash
ldd payroll-and-monitoring-system
# Look for "not found" entries
# Install missing libraries via package manager
```

**Wrong architecture:**
```bash
file payroll-and-monitoring-system
# Should show:
# x64: "ELF 64-bit LSB executable, x86-64"
# ARM64: "ELF 64-bit LSB executable, aarch64"
```

**GLIBC version mismatch:**
```bash
# Binary built on newer system may not run on older
# Solution: Build on oldest target distribution
```

### Display Issues

**No display:**
```bash
# Make sure X11 is running
echo $DISPLAY

# Or use Wayland
# Check if Wayland is supported
```

**OpenGL errors:**
```bash
# Install OpenGL drivers
# Ubuntu/Debian:
sudo apt-get install mesa-utils
glxinfo | grep "OpenGL version"
```

## Advanced Options

### Static Linking (Not Recommended)

For a fully static binary (no shared library dependencies):

```yaml
# Add to CMake configuration
-DCMAKE_EXE_LINKER_FLAGS="-static"
```

**Drawbacks:**
- Much larger binary size
- May break on some systems
- Not recommended for Linux

### Cross-Compilation

The workflow supports native builds on ARM64 runners:
- x64: Builds natively on x64 runner
- ARM64: Builds on ARM64 runner (if available) or cross-compiles

### Custom Distribution

To add a new distribution, edit the workflow:

```yaml
distribution:
  type: choice
  options:
    - your-custom-distro:latest
```

## Distribution-Specific Notes

### Ubuntu

- **Recommended for most users**
- LTS versions get 5 years of support
- Largest package repository
- Best hardware support
- Most third-party software targets Ubuntu

### Debian

- **Maximum stability**
- Conservative package updates
- Longer release cycles (2 years)
- Great for servers and Raspberry Pi
- Rock-solid reliability

### Ubuntu Derivatives

**Linux Mint:**
- Based on Ubuntu LTS
- Use the corresponding Ubuntu build
- Mint 21.x → Use Ubuntu 22.04 build
- Mint 20.x → Use Ubuntu 20.04 build

**Pop!_OS:**
- Based on Ubuntu
- Use the matching Ubuntu version build
- Pop!_OS 22.04 → Use Ubuntu 22.04 build

**elementary OS:**
- Based on Ubuntu LTS
- elementary OS 7.x → Use Ubuntu 22.04 build
- elementary OS 6.x → Use Ubuntu 20.04 build

**Raspberry Pi OS:**
- Based on Debian
- Use Debian 12 (arm64) for 64-bit Pi OS
- Use Debian 11 (arm64) for older Pi OS versions

## Which Distribution to Choose?

**For end users:**
- Use Ubuntu 22.04 LTS (most compatible)
- Works on most Debian/Ubuntu derivatives

**For developers:**
- Use your development distribution's matching version

**For servers:**
- Use Debian Stable or Ubuntu LTS

**For Raspberry Pi:**
- Use Debian 12 arm64 for Pi 4/5 (64-bit)
- Check your Pi OS version and match Debian version

## ARM64 Considerations

**Performance:**
- Native ARM64 build performs well on ARM hardware
- No emulation overhead

**Compatibility:**
- Raspberry Pi 4/5 (64-bit OS required)
- AWS Graviton instances
- Oracle Cloud ARM instances
- Apple Silicon via Docker/Lima (not recommended, use macOS workflow)

**Limitations:**
- Fewer ARM64 runners available
- May use QEMU emulation (slower)
- Some packages may not be available

## Best Practices

1. **Choose the right distribution:**
   - Match your deployment target
   - Ubuntu for widest compatibility

2. **Test on target distribution:**
   - Don't assume cross-distribution compatibility
   - Verify dependencies are available

3. **Use LTS releases for production:**
   - Ubuntu 22.04 or 20.04
   - Longer support period

4. **Include installation instructions:**
   - List required dependencies
   - Provide install commands

## Performance Comparison

| Distribution | Build Speed | Binary Size | Optimization | Package Updates |
|--------------|-------------|-------------|--------------|-----------------|
| Ubuntu 22.04 | Fast | Medium | Good | Frequent (LTS) |
| Ubuntu 20.04 | Fast | Medium | Good | Regular (LTS) |
| Debian 12 | Fast | Medium | Good | Stable releases |
| Debian 11 | Medium | Medium | Good | Conservative |

## Support

If you encounter issues:
1. Check the workflow run logs
2. Verify distribution and architecture
3. Check dependency installation logs
4. Test on actual target system

---

**Quick Start:** Actions → Build Linux (Manual) → Select distribution + architecture → Run workflow

**Remember:** Test the binary on your actual target distribution and architecture!

